
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

#include "kafs_locl.h"

struct krb5_kafs_data {
    krb5_context context;
    krb5_ccache id;
    krb5_const_realm realm;
};

enum {
    KAFS_RXKAD_2B_KVNO = 213,
    KAFS_RXKAD_K5_KVNO = 256
};

static int
v5_to_kt(krb5_creds *cred, uid_t uid, struct kafs_token *kt, int local524)
{
    int kvno, ret;

    kt->ticket = NULL;

    /* check if des key */
    if (cred->session.keyvalue.length != 8)
	return EINVAL;

    if (local524) {
	Ticket t;
	unsigned char *buf;
	size_t buf_len;
	size_t len;

	kvno = KAFS_RXKAD_2B_KVNO;

	ret = decode_Ticket(cred->ticket.data, cred->ticket.length, &t, &len);
	if (ret)
	    return ret;
	if (t.tkt_vno != 5)
	    return -1;

	ASN1_MALLOC_ENCODE(EncryptedData, buf, buf_len, &t.enc_part,
			   &len, ret);
	free_Ticket(&t);
	if (ret)
	    return ret;
	if(buf_len != len) {
	    free(buf);
	    return KRB5KRB_ERR_GENERIC;
	}

	kt->ticket = buf;
	kt->ticket_len = buf_len;

    } else {
	kvno = KAFS_RXKAD_K5_KVNO;
	kt->ticket = malloc(cred->ticket.length);
	if (kt->ticket == NULL)
	    return ENOMEM;
	kt->ticket_len = cred->ticket.length;
	memcpy(kt->ticket, cred->ticket.data, kt->ticket_len);

	ret = 0;
    }


    /*
     * Build a struct ClearToken
     */

    kt->ct.AuthHandle = kvno;
    memcpy(kt->ct.HandShakeKey, cred->session.keyvalue.data, 8);
    kt->ct.ViceId = uid;
    kt->ct.BeginTimestamp = cred->times.starttime;
    kt->ct.EndTimestamp = cred->times.endtime;

    _kafs_fixup_viceid(&kt->ct, uid);

    return 0;
}

static krb5_error_code
v5_convert(krb5_context context, krb5_ccache id,
	   krb5_creds *cred, uid_t uid,
	   const char *cell,
	   struct kafs_token *kt)
{
    krb5_error_code ret;
    char *c, *val;

    c = strdup(cell);
    if (c == NULL)
	return ENOMEM;
    _kafs_foldup(c, c);
    krb5_appdefault_string (context, "libkafs",
			    c,
			    "afs-use-524", "2b", &val);
    free(c);

    if (strcasecmp(val, "local") == 0 ||
	strcasecmp(val, "2b") == 0)
	ret = v5_to_kt(cred, uid, kt, 1);
    else
	ret = v5_to_kt(cred, uid, kt, 0);

    free(val);
    return ret;
}


/*
 *
 */

static int
get_cred(struct kafs_data *data, const char *name, const char *inst,
	 const char *realm, uid_t uid, struct kafs_token *kt)
{
    krb5_error_code ret;
    krb5_creds in_creds, *out_creds;
    struct krb5_kafs_data *d = data->data;
    int invalid;

    memset(&in_creds, 0, sizeof(in_creds));

    ret = krb5_make_principal(d->context, &in_creds.server,
			      realm, name, inst, NULL);
    if(ret)
	return ret;
    ret = krb5_cc_get_principal(d->context, d->id, &in_creds.client);
    if(ret){
	krb5_free_principal(d->context, in_creds.server);
	return ret;
    }

    in_creds.session.keytype = ETYPE_DES_CBC_CRC;

    /* check if des is disable, and in that case enable it for afs */
    invalid = krb5_enctype_valid(d->context, in_creds.session.keytype);
    if (invalid)
	krb5_enctype_enable(d->context, in_creds.session.keytype);

    ret = krb5_get_credentials(d->context, 0, d->id, &in_creds, &out_creds);
    if (ret) {
	in_creds.session.keytype = ETYPE_DES_CBC_MD5;
	ret = krb5_get_credentials(d->context, 0, d->id, &in_creds, &out_creds);
    }

    if (invalid)
	krb5_enctype_disable(d->context, in_creds.session.keytype);

    krb5_free_principal(d->context, in_creds.server);
    krb5_free_principal(d->context, in_creds.client);
    if(ret)
	return ret;

    ret = v5_convert(d->context, d->id, out_creds, uid,
		     (inst != NULL && inst[0] != '\0') ? inst : realm, kt);
    krb5_free_creds(d->context, out_creds);

    return ret;
}

static const char *
get_error(struct kafs_data *data, int error)
{
    struct krb5_kafs_data *d = data->data;
    return krb5_get_error_message(d->context, error);
}

static void
free_error(struct kafs_data *data, const char *str)
{
    struct krb5_kafs_data *d = data->data;
    krb5_free_error_message(d->context, str);
}

static krb5_error_code
afslog_uid_int(struct kafs_data *data, const char *cell, const char *rh,
	       uid_t uid, const char *homedir)
{
    krb5_error_code ret;
    struct kafs_token kt;
    krb5_principal princ;
    const char *trealm; /* ticket realm */
    struct krb5_kafs_data *d = data->data;

    if (cell == 0 || cell[0] == 0)
	return _kafs_afslog_all_local_cells (data, uid, homedir);

    ret = krb5_cc_get_principal (d->context, d->id, &princ);
    if (ret)
	return ret;

    trealm = krb5_principal_get_realm (d->context, princ);

    kt.ticket = NULL;
    ret = _kafs_get_cred(data, cell, d->realm, trealm, uid, &kt);
    krb5_free_principal (d->context, princ);

    if(ret == 0) {
	ret = kafs_settoken_rxkad(cell, &kt.ct, kt.ticket, kt.ticket_len);
	free(kt.ticket);
    }
    return ret;
}

static char *
get_realm(struct kafs_data *data, const char *host)
{
    struct krb5_kafs_data *d = data->data;
    krb5_realm *realms;
    char *r;
    if(krb5_get_host_realm(d->context, host, &realms))
	return NULL;
    r = strdup(realms[0]);
    krb5_free_host_realm(d->context, realms);
    return r;
}

krb5_error_code
krb5_afslog_uid_home(krb5_context context,
		     krb5_ccache id,
		     const char *cell,
		     krb5_const_realm realm,
		     uid_t uid,
		     const char *homedir)
{
    struct kafs_data kd;
    struct krb5_kafs_data d;
    krb5_error_code ret;

    kd.name = "krb5";
    kd.afslog_uid = afslog_uid_int;
    kd.get_cred = get_cred;
    kd.get_realm = get_realm;
    kd.get_error = get_error;
    kd.free_error = free_error;
    kd.data = &d;
    if (context == NULL) {
	ret = krb5_init_context(&d.context);
	if (ret)
	    return ret;
    } else
	d.context = context;
    if (id == NULL) {
	ret = krb5_cc_default(d.context, &d.id);
	if (ret)
	    goto out;
    } else
	d.id = id;
    d.realm = realm;
    ret = afslog_uid_int(&kd, cell, 0, uid, homedir);
    if (id == NULL)
	krb5_cc_close(context, d.id);
 out:
    if (context == NULL)
	krb5_free_context(d.context);
    return ret;
}

krb5_error_code
krb5_afslog_uid(krb5_context context,
		krb5_ccache id,
		const char *cell,
		krb5_const_realm realm,
		uid_t uid)
{
    return krb5_afslog_uid_home (context, id, cell, realm, uid, NULL);
}

krb5_error_code
krb5_afslog(krb5_context context,
	    krb5_ccache id,
	    const char *cell,
	    krb5_const_realm realm)
{
    return krb5_afslog_uid (context, id, cell, realm, getuid());
}

krb5_error_code
krb5_afslog_home(krb5_context context,
		 krb5_ccache id,
		 const char *cell,
		 krb5_const_realm realm,
		 const char *homedir)
{
    return krb5_afslog_uid_home (context, id, cell, realm, getuid(), homedir);
}

/*
 *
 */

krb5_error_code
krb5_realm_of_cell(const char *cell, char **realm)
{
    struct kafs_data kd;

    kd.name = "krb5";
    kd.get_realm = get_realm;
    kd.get_error = get_error;
    kd.free_error = free_error;
    return _kafs_realm_of_cell(&kd, cell, realm);
}

/*
 *
 */

int
kafs_settoken5(krb5_context context, const char *cell, uid_t uid,
	       krb5_creds *cred)
{
    struct kafs_token kt;
    int ret;

    ret = v5_convert(context, NULL, cred, uid, cell, &kt);
    if (ret)
	return ret;

    ret = kafs_settoken_rxkad(cell, &kt.ct, kt.ticket, kt.ticket_len);

    free(kt.ticket);

    return ret;
}