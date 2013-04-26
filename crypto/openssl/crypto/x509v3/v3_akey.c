
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

#include <stdio.h>
#include "cryptlib.h"
#include <openssl/conf.h>
#include <openssl/asn1.h>
#include <openssl/asn1t.h>
#include <openssl/x509v3.h>

static STACK_OF(CONF_VALUE) *i2v_AUTHORITY_KEYID(X509V3_EXT_METHOD *method,
			AUTHORITY_KEYID *akeyid, STACK_OF(CONF_VALUE) *extlist);
static AUTHORITY_KEYID *v2i_AUTHORITY_KEYID(X509V3_EXT_METHOD *method,
			X509V3_CTX *ctx, STACK_OF(CONF_VALUE) *values);

const X509V3_EXT_METHOD v3_akey_id =
	{
	NID_authority_key_identifier,
	X509V3_EXT_MULTILINE, ASN1_ITEM_ref(AUTHORITY_KEYID),
	0,0,0,0,
	0,0,
	(X509V3_EXT_I2V)i2v_AUTHORITY_KEYID,
	(X509V3_EXT_V2I)v2i_AUTHORITY_KEYID,
	0,0,
	NULL
	};

static STACK_OF(CONF_VALUE) *i2v_AUTHORITY_KEYID(X509V3_EXT_METHOD *method,
	     AUTHORITY_KEYID *akeyid, STACK_OF(CONF_VALUE) *extlist)
{
	char *tmp;
	if(akeyid->keyid) {
		tmp = hex_to_string(akeyid->keyid->data, akeyid->keyid->length);
		X509V3_add_value("keyid", tmp, &extlist);
		OPENSSL_free(tmp);
	}
	if(akeyid->issuer) 
		extlist = i2v_GENERAL_NAMES(NULL, akeyid->issuer, extlist);
	if(akeyid->serial) {
		tmp = hex_to_string(akeyid->serial->data,
						 akeyid->serial->length);
		X509V3_add_value("serial", tmp, &extlist);
		OPENSSL_free(tmp);
	}
	return extlist;
}

/* Currently two options:
 * keyid: use the issuers subject keyid, the value 'always' means its is
 * an error if the issuer certificate doesn't have a key id.
 * issuer: use the issuers cert issuer and serial number. The default is
 * to only use this if keyid is not present. With the option 'always'
 * this is always included.
 */

static AUTHORITY_KEYID *v2i_AUTHORITY_KEYID(X509V3_EXT_METHOD *method,
	     X509V3_CTX *ctx, STACK_OF(CONF_VALUE) *values)
	{
	char keyid=0, issuer=0;
	int i;
	CONF_VALUE *cnf;
	ASN1_OCTET_STRING *ikeyid = NULL;
	X509_NAME *isname = NULL;
	GENERAL_NAMES * gens = NULL;
	GENERAL_NAME *gen = NULL;
	ASN1_INTEGER *serial = NULL;
	X509_EXTENSION *ext;
	X509 *cert;
	AUTHORITY_KEYID *akeyid;

	for(i = 0; i < sk_CONF_VALUE_num(values); i++)
		{
		cnf = sk_CONF_VALUE_value(values, i);
		if(!strcmp(cnf->name, "keyid"))
			{
			keyid = 1;
			if(cnf->value && !strcmp(cnf->value, "always"))
				keyid = 2;
			}
		else if(!strcmp(cnf->name, "issuer"))
			{
			issuer = 1;
			if(cnf->value && !strcmp(cnf->value, "always"))
				issuer = 2;
			}
		else
			{
			X509V3err(X509V3_F_V2I_AUTHORITY_KEYID,X509V3_R_UNKNOWN_OPTION);
			ERR_add_error_data(2, "name=", cnf->name);
			return NULL;
			}
		}

	if(!ctx || !ctx->issuer_cert)
		{
		if(ctx && (ctx->flags==CTX_TEST))
			return AUTHORITY_KEYID_new();
		X509V3err(X509V3_F_V2I_AUTHORITY_KEYID,X509V3_R_NO_ISSUER_CERTIFICATE);
		return NULL;
		}

	cert = ctx->issuer_cert;

	if(keyid)
		{
		i = X509_get_ext_by_NID(cert, NID_subject_key_identifier, -1);
		if((i >= 0)  && (ext = X509_get_ext(cert, i)))
			ikeyid = X509V3_EXT_d2i(ext);
		if(keyid==2 && !ikeyid)
			{
			X509V3err(X509V3_F_V2I_AUTHORITY_KEYID,X509V3_R_UNABLE_TO_GET_ISSUER_KEYID);
			return NULL;
			}
		}

	if((issuer && !ikeyid) || (issuer == 2))
		{
		isname = X509_NAME_dup(X509_get_issuer_name(cert));
		serial = M_ASN1_INTEGER_dup(X509_get_serialNumber(cert));
		if(!isname || !serial)
			{
			X509V3err(X509V3_F_V2I_AUTHORITY_KEYID,X509V3_R_UNABLE_TO_GET_ISSUER_DETAILS);
			goto err;
			}
		}

	if(!(akeyid = AUTHORITY_KEYID_new())) goto err;

	if(isname)
		{
		if(!(gens = sk_GENERAL_NAME_new_null())
			|| !(gen = GENERAL_NAME_new())
			|| !sk_GENERAL_NAME_push(gens, gen))
			{
			X509V3err(X509V3_F_V2I_AUTHORITY_KEYID,ERR_R_MALLOC_FAILURE);
			goto err;
			}
		gen->type = GEN_DIRNAME;
		gen->d.dirn = isname;
		}

	akeyid->issuer = gens;
	akeyid->serial = serial;
	akeyid->keyid = ikeyid;

	return akeyid;

 err:
	X509_NAME_free(isname);
	M_ASN1_INTEGER_free(serial);
	M_ASN1_OCTET_STRING_free(ikeyid);
	return NULL;
	}