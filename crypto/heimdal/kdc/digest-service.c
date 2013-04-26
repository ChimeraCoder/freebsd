
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define HC_DEPRECATED_CRYPTO

#include "headers.h"
#include <digest_asn1.h>
#include <heimntlm.h>
#include <heim-ipc.h>
#include <getarg.h>

typedef struct pk_client_params pk_client_params;
struct DigestREQ;
struct Kx509Request;
#include <kdc-private.h>

krb5_kdc_configuration *config;

static void
ntlm_service(void *ctx, const heim_idata *req,
	     const heim_icred cred,
	     heim_ipc_complete complete,
	     heim_sipc_call cctx)
{
    NTLMRequest2 ntq;
    unsigned char sessionkey[16];
    heim_idata rep = { 0, NULL };
    krb5_context context = ctx;
    hdb_entry_ex *user = NULL;
    Key *key = NULL;
    NTLMReply ntp;
    size_t size;
    int ret;
    const char *domain;

    kdc_log(context, config, 1, "digest-request: uid=%d",
	    (int)heim_ipc_cred_get_uid(cred));

    if (heim_ipc_cred_get_uid(cred) != 0) {
	(*complete)(cctx, EPERM, NULL);
	return;
    }

    ntp.success = 0;
    ntp.flags = 0;
    ntp.sessionkey = NULL;

    ret = decode_NTLMRequest2(req->data, req->length, &ntq, NULL);
    if (ret)
	goto failed;

    /* XXX forward to NetrLogonSamLogonEx() if not a local domain */
    if (strcmp(ntq.loginDomainName, "BUILTIN") == 0) {
	domain = ntq.loginDomainName;
    } else if (strcmp(ntq.loginDomainName, "") == 0) {
	domain = "BUILTIN";
    } else {
	ret = EINVAL;
	goto failed;
    }

    kdc_log(context, config, 1, "digest-request: user=%s/%s",
	    ntq.loginUserName, domain);

    if (ntq.lmchallenge.length != 8)
	goto failed;

    if (ntq.ntChallengeResponce.length == 0)
	goto failed;

    {
	krb5_principal client;

	ret = krb5_make_principal(context, &client, domain,
				  ntq.loginUserName, NULL);
	if (ret)
	    goto failed;

	krb5_principal_set_type(context, client, KRB5_NT_NTLM);

	ret = _kdc_db_fetch(context, config, client,
			    HDB_F_GET_CLIENT, NULL, NULL, &user);
	krb5_free_principal(context, client);
	if (ret)
	    goto failed;

	ret = hdb_enctype2key(context, &user->entry,
			      ETYPE_ARCFOUR_HMAC_MD5, &key);
	if (ret) {
	    krb5_set_error_message(context, ret, "NTLM missing arcfour key");
	    goto failed;
	}
    }

    kdc_log(context, config, 2,
	    "digest-request: found user, processing ntlm request", ret);

    if (ntq.ntChallengeResponce.length != 24) {
	struct ntlm_buf infotarget, answer;

	answer.length = ntq.ntChallengeResponce.length;
	answer.data = ntq.ntChallengeResponce.data;

	ret = heim_ntlm_verify_ntlm2(key->key.keyvalue.data,
				     key->key.keyvalue.length,
				     ntq.loginUserName,
				     ntq.loginDomainName,
				     0,
				     ntq.lmchallenge.data,
				     &answer,
				     &infotarget,
				     sessionkey);
	if (ret) {
	    goto failed;
	}

	free(infotarget.data);
	/* XXX verify info target */

    } else {
	struct ntlm_buf answer;

	if (ntq.flags & NTLM_NEG_NTLM2_SESSION) {
	    unsigned char sessionhash[MD5_DIGEST_LENGTH];
	    EVP_MD_CTX *md5ctx;

	    /* the first first 8 bytes is the challenge, what is the other 16 bytes ? */
	    if (ntq.lmChallengeResponce.length != 24)
		goto failed;

	    md5ctx = EVP_MD_CTX_create();
	    EVP_DigestInit_ex(md5ctx, EVP_md5(), NULL);
	    EVP_DigestUpdate(md5ctx, ntq.lmchallenge.data, 8);
	    EVP_DigestUpdate(md5ctx, ntq.lmChallengeResponce.data, 8);
	    EVP_DigestFinal_ex(md5ctx, sessionhash, NULL);
	    EVP_MD_CTX_destroy(md5ctx);
	    memcpy(ntq.lmchallenge.data, sessionhash, ntq.lmchallenge.length);
	}

	ret = heim_ntlm_calculate_ntlm1(key->key.keyvalue.data,
					key->key.keyvalue.length,
					ntq.lmchallenge.data, &answer);
	if (ret)
	    goto failed;

	if (ntq.ntChallengeResponce.length != answer.length ||
	    memcmp(ntq.ntChallengeResponce.data, answer.data, answer.length) != 0) {
	    free(answer.data);
	    ret = EINVAL;
	    goto failed;
	}
	free(answer.data);

	{
	    EVP_MD_CTX *ctxp;

	    ctxp = EVP_MD_CTX_create();
	    EVP_DigestInit_ex(ctxp, EVP_md4(), NULL);
	    EVP_DigestUpdate(ctxp, key->key.keyvalue.data, key->key.keyvalue.length);
	    EVP_DigestFinal_ex(ctxp, sessionkey, NULL);
	    EVP_MD_CTX_destroy(ctxp);
	}
    }

    ntp.success = 1;

    ASN1_MALLOC_ENCODE(NTLMReply, rep.data, rep.length, &ntp, &size, ret);
    if (ret)
	goto failed;
    if (rep.length != size)
	abort();

  failed:
    kdc_log(context, config, 1, "digest-request: %d", ret);

    (*complete)(cctx, ret, &rep);

    free(rep.data);

    free_NTLMRequest2(&ntq);
    if (user)
	_kdc_free_ent (context, user);
}

static int help_flag;
static int version_flag;

static struct getargs args[] = {
    {	"help",		'h',	arg_flag,   &help_flag, NULL, NULL },
    {	"version",	'v',	arg_flag,   &version_flag, NULL, NULL }
};

static int num_args = sizeof(args) / sizeof(args[0]);

static void
usage(int ret)
{
    arg_printusage (args, num_args, NULL, "");
    exit (ret);
}

int
main(int argc, char **argv)
{
    krb5_context context;
    int ret, optidx = 0;

    setprogname(argv[0]);

    if (getarg(args, num_args, argc, argv, &optidx))
	usage(1);

    if (help_flag)
	usage(0);

    if (version_flag) {
	print_version(NULL);
	exit(0);
    }

    ret = krb5_init_context(&context);
    if (ret)
	krb5_errx(context, 1, "krb5_init_context");

    ret = krb5_kdc_get_config(context, &config);
    if (ret)
	krb5_err(context, 1, ret, "krb5_kdc_default_config");

    kdc_openlog(context, "digest-service", config);

    ret = krb5_kdc_set_dbinfo(context, config);
    if (ret)
	krb5_err(context, 1, ret, "krb5_kdc_set_dbinfo");

#if __APPLE__
    {
	heim_sipc mach;
	heim_sipc_launchd_mach_init("org.h5l.ntlm-service",
				    ntlm_service, context, &mach);
	heim_sipc_timeout(60);
    }
#endif
    {
	heim_sipc un;
	heim_sipc_service_unix("org.h5l.ntlm-service", ntlm_service, NULL, &un);
    }

    heim_ipc_main();
    return 0;
}