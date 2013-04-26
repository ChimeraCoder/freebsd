
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

#include "eng_int.h"
#include "asn1_locl.h"
#include <openssl/evp.h>

/* If this symbol is defined then ENGINE_get_pkey_asn1_meth_engine(), the
 * function that is used by EVP to hook in pkey_asn1_meth code and cache
 * defaults (etc), will display brief debugging summaries to stderr with the
 * 'nid'. */
/* #define ENGINE_PKEY_ASN1_METH_DEBUG */

static ENGINE_TABLE *pkey_asn1_meth_table = NULL;

void ENGINE_unregister_pkey_asn1_meths(ENGINE *e)
	{
	engine_table_unregister(&pkey_asn1_meth_table, e);
	}

static void engine_unregister_all_pkey_asn1_meths(void)
	{
	engine_table_cleanup(&pkey_asn1_meth_table);
	}

int ENGINE_register_pkey_asn1_meths(ENGINE *e)
	{
	if(e->pkey_asn1_meths)
		{
		const int *nids;
		int num_nids = e->pkey_asn1_meths(e, NULL, &nids, 0);
		if(num_nids > 0)
			return engine_table_register(&pkey_asn1_meth_table,
				engine_unregister_all_pkey_asn1_meths, e, nids,
					num_nids, 0);
		}
	return 1;
	}

void ENGINE_register_all_pkey_asn1_meths(void)
	{
	ENGINE *e;

	for(e=ENGINE_get_first() ; e ; e=ENGINE_get_next(e))
		ENGINE_register_pkey_asn1_meths(e);
	}

int ENGINE_set_default_pkey_asn1_meths(ENGINE *e)
	{
	if(e->pkey_asn1_meths)
		{
		const int *nids;
		int num_nids = e->pkey_asn1_meths(e, NULL, &nids, 0);
		if(num_nids > 0)
			return engine_table_register(&pkey_asn1_meth_table,
				engine_unregister_all_pkey_asn1_meths, e, nids,
					num_nids, 1);
		}
	return 1;
	}

/* Exposed API function to get a functional reference from the implementation
 * table (ie. try to get a functional reference from the tabled structural
 * references) for a given pkey_asn1_meth 'nid' */
ENGINE *ENGINE_get_pkey_asn1_meth_engine(int nid)
	{
	return engine_table_select(&pkey_asn1_meth_table, nid);
	}

/* Obtains a pkey_asn1_meth implementation from an ENGINE functional reference */
const EVP_PKEY_ASN1_METHOD *ENGINE_get_pkey_asn1_meth(ENGINE *e, int nid)
	{
	EVP_PKEY_ASN1_METHOD *ret;
	ENGINE_PKEY_ASN1_METHS_PTR fn = ENGINE_get_pkey_asn1_meths(e);
	if(!fn || !fn(e, &ret, NULL, nid))
		{
		ENGINEerr(ENGINE_F_ENGINE_GET_PKEY_ASN1_METH,
				ENGINE_R_UNIMPLEMENTED_PUBLIC_KEY_METHOD);
		return NULL;
		}
	return ret;
	}

/* Gets the pkey_asn1_meth callback from an ENGINE structure */
ENGINE_PKEY_ASN1_METHS_PTR ENGINE_get_pkey_asn1_meths(const ENGINE *e)
	{
	return e->pkey_asn1_meths;
	}

/* Sets the pkey_asn1_meth callback in an ENGINE structure */
int ENGINE_set_pkey_asn1_meths(ENGINE *e, ENGINE_PKEY_ASN1_METHS_PTR f)
	{
	e->pkey_asn1_meths = f;
	return 1;
	}

/* Internal function to free up EVP_PKEY_ASN1_METHOD structures before an
 * ENGINE is destroyed
 */

void engine_pkey_asn1_meths_free(ENGINE *e)
	{
	int i;
	EVP_PKEY_ASN1_METHOD *pkm;
	if (e->pkey_asn1_meths)
		{
		const int *pknids;
		int npknids;
		npknids = e->pkey_asn1_meths(e, NULL, &pknids, 0);
		for (i = 0; i < npknids; i++)
			{
			if (e->pkey_asn1_meths(e, &pkm, NULL, pknids[i]))
				{
				EVP_PKEY_asn1_free(pkm);
				}
			}
		}
	}

/* Find a method based on a string. This does a linear search through
 * all implemented algorithms. This is OK in practice because only
 * a small number of algorithms are likely to be implemented in an engine
 * and it is not used for speed critical operations.
 */

const EVP_PKEY_ASN1_METHOD *ENGINE_get_pkey_asn1_meth_str(ENGINE *e,
					const char *str, int len)
	{
	int i, nidcount;
	const int *nids;
	EVP_PKEY_ASN1_METHOD *ameth;
	if (!e->pkey_asn1_meths)
		return NULL;
	if (len == -1)
		len = strlen(str);
	nidcount = e->pkey_asn1_meths(e, NULL, &nids, 0);
	for (i = 0; i < nidcount; i++)
		{
		e->pkey_asn1_meths(e, &ameth, NULL, nids[i]);
		if (((int)strlen(ameth->pem_str) == len) && 
					!strncasecmp(ameth->pem_str, str, len))
			return ameth;
		}
	return NULL;
	}

typedef struct 
	{
	ENGINE *e;
	const EVP_PKEY_ASN1_METHOD *ameth;
	const char *str;
	int len;
	} ENGINE_FIND_STR;

static void look_str_cb(int nid, STACK_OF(ENGINE) *sk, ENGINE *def, void *arg)
	{
	ENGINE_FIND_STR *lk = arg;
	int i;
	if (lk->ameth)
		return;
	for (i = 0; i < sk_ENGINE_num(sk); i++)
		{
		ENGINE *e = sk_ENGINE_value(sk, i);
		EVP_PKEY_ASN1_METHOD *ameth;
		e->pkey_asn1_meths(e, &ameth, NULL, nid);
		if (((int)strlen(ameth->pem_str) == lk->len) && 
				!strncasecmp(ameth->pem_str, lk->str, lk->len))
			{
			lk->e = e;
			lk->ameth = ameth;
			return;
			}
		}
	}

const EVP_PKEY_ASN1_METHOD *ENGINE_pkey_asn1_find_str(ENGINE **pe,
					const char *str, int len)
	{
	ENGINE_FIND_STR fstr;
	fstr.e = NULL;
	fstr.ameth = NULL;
	fstr.str = str;
	fstr.len = len;
	CRYPTO_w_lock(CRYPTO_LOCK_ENGINE);
	engine_table_doall(pkey_asn1_meth_table, look_str_cb, &fstr);
	/* If found obtain a structural reference to engine */
	if (fstr.e)
		{
		fstr.e->struct_ref++;
		engine_ref_debug(fstr.e, 0, 1)
		}
	*pe = fstr.e;
	CRYPTO_w_unlock(CRYPTO_LOCK_ENGINE);
	return fstr.ameth;
	}