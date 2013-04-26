
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

#include "includes.h"

#include <sys/types.h>

#include <pwd.h>
#include <string.h>
#include <stdarg.h>

#include "xmalloc.h"
#include "ssh2.h"
#include "packet.h"
#include "buffer.h"
#include "log.h"
#include "servconf.h"
#include "compat.h"
#include "key.h"
#include "hostfile.h"
#include "auth.h"
#include "canohost.h"
#ifdef GSSAPI
#include "ssh-gss.h"
#endif
#include "monitor_wrap.h"
#include "pathnames.h"

/* import */
extern ServerOptions options;
extern u_char *session_id2;
extern u_int session_id2_len;

static int
userauth_hostbased(Authctxt *authctxt)
{
	Buffer b;
	Key *key = NULL;
	char *pkalg, *cuser, *chost, *service;
	u_char *pkblob, *sig;
	u_int alen, blen, slen;
	int pktype;
	int authenticated = 0;

	if (!authctxt->valid) {
		debug2("userauth_hostbased: disabled because of invalid user");
		return 0;
	}
	pkalg = packet_get_string(&alen);
	pkblob = packet_get_string(&blen);
	chost = packet_get_string(NULL);
	cuser = packet_get_string(NULL);
	sig = packet_get_string(&slen);

	debug("userauth_hostbased: cuser %s chost %s pkalg %s slen %d",
	    cuser, chost, pkalg, slen);
#ifdef DEBUG_PK
	debug("signature:");
	buffer_init(&b);
	buffer_append(&b, sig, slen);
	buffer_dump(&b);
	buffer_free(&b);
#endif
	pktype = key_type_from_name(pkalg);
	if (pktype == KEY_UNSPEC) {
		/* this is perfectly legal */
		logit("userauth_hostbased: unsupported "
		    "public key algorithm: %s", pkalg);
		goto done;
	}
	key = key_from_blob(pkblob, blen);
	if (key == NULL) {
		error("userauth_hostbased: cannot decode key: %s", pkalg);
		goto done;
	}
	if (key->type != pktype) {
		error("userauth_hostbased: type mismatch for decoded key "
		    "(received %d, expected %d)", key->type, pktype);
		goto done;
	}
	service = datafellows & SSH_BUG_HBSERVICE ? "ssh-userauth" :
	    authctxt->service;
	buffer_init(&b);
	buffer_put_string(&b, session_id2, session_id2_len);
	/* reconstruct packet */
	buffer_put_char(&b, SSH2_MSG_USERAUTH_REQUEST);
	buffer_put_cstring(&b, authctxt->user);
	buffer_put_cstring(&b, service);
	buffer_put_cstring(&b, "hostbased");
	buffer_put_string(&b, pkalg, alen);
	buffer_put_string(&b, pkblob, blen);
	buffer_put_cstring(&b, chost);
	buffer_put_cstring(&b, cuser);
#ifdef DEBUG_PK
	buffer_dump(&b);
#endif
	/* test for allowed key and correct signature */
	authenticated = 0;
	if (PRIVSEP(hostbased_key_allowed(authctxt->pw, cuser, chost, key)) &&
	    PRIVSEP(key_verify(key, sig, slen, buffer_ptr(&b),
			buffer_len(&b))) == 1)
		authenticated = 1;

	buffer_free(&b);
done:
	debug2("userauth_hostbased: authenticated %d", authenticated);
	if (key != NULL)
		key_free(key);
	xfree(pkalg);
	xfree(pkblob);
	xfree(cuser);
	xfree(chost);
	xfree(sig);
	return authenticated;
}

/* return 1 if given hostkey is allowed */
int
hostbased_key_allowed(struct passwd *pw, const char *cuser, char *chost,
    Key *key)
{
	const char *resolvedname, *ipaddr, *lookup, *reason;
	HostStatus host_status;
	int len;
	char *fp;

	if (auth_key_is_revoked(key))
		return 0;

	resolvedname = get_canonical_hostname(options.use_dns);
	ipaddr = get_remote_ipaddr();

	debug2("userauth_hostbased: chost %s resolvedname %s ipaddr %s",
	    chost, resolvedname, ipaddr);

	if (((len = strlen(chost)) > 0) && chost[len - 1] == '.') {
		debug2("stripping trailing dot from chost %s", chost);
		chost[len - 1] = '\0';
	}

	if (options.hostbased_uses_name_from_packet_only) {
		if (auth_rhosts2(pw, cuser, chost, chost) == 0)
			return 0;
		lookup = chost;
	} else {
		if (strcasecmp(resolvedname, chost) != 0)
			logit("userauth_hostbased mismatch: "
			    "client sends %s, but we resolve %s to %s",
			    chost, ipaddr, resolvedname);
		if (auth_rhosts2(pw, cuser, resolvedname, ipaddr) == 0)
			return 0;
		lookup = resolvedname;
	}
	debug2("userauth_hostbased: access allowed by auth_rhosts2");

	if (key_is_cert(key) && 
	    key_cert_check_authority(key, 1, 0, lookup, &reason)) {
		error("%s", reason);
		auth_debug_add("%s", reason);
		return 0;
	}

	host_status = check_key_in_hostfiles(pw, key, lookup,
	    _PATH_SSH_SYSTEM_HOSTFILE,
	    options.ignore_user_known_hosts ? NULL : _PATH_SSH_USER_HOSTFILE);

	/* backward compat if no key has been found. */
	if (host_status == HOST_NEW) {
		host_status = check_key_in_hostfiles(pw, key, lookup,
		    _PATH_SSH_SYSTEM_HOSTFILE2,
		    options.ignore_user_known_hosts ? NULL :
		    _PATH_SSH_USER_HOSTFILE2);
	}

	if (host_status == HOST_OK) {
		if (key_is_cert(key)) {
			fp = key_fingerprint(key->cert->signature_key,
			    SSH_FP_MD5, SSH_FP_HEX);
			verbose("Accepted certificate ID \"%s\" signed by "
			    "%s CA %s from %s@%s", key->cert->key_id,
			    key_type(key->cert->signature_key), fp,
			    cuser, lookup);
		} else {
			fp = key_fingerprint(key, SSH_FP_MD5, SSH_FP_HEX);
			verbose("Accepted %s public key %s from %s@%s",
			    key_type(key), fp, cuser, lookup);
		}
		xfree(fp);
	}

	return (host_status == HOST_OK);
}

Authmethod method_hostbased = {
	"hostbased",
	userauth_hostbased,
	&options.hostbased_authentication
};