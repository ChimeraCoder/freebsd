
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

/* If this symbol is defined then ENGINE_get_cipher_engine(), the function that
 * is used by EVP to hook in cipher code and cache defaults (etc), will display
 * brief debugging summaries to stderr with the 'nid'. */
/* #define ENGINE_CIPHER_DEBUG */

static ENGINE_TABLE *cipher_table = NULL;

void ENGINE_unregister_ciphers(ENGINE *e)
	{
	engine_table_unregister(&cipher_table, e);
	}

static void engine_unregister_all_ciphers(void)
	{
	engine_table_cleanup(&cipher_table);
	}

int ENGINE_register_ciphers(ENGINE *e)
	{
	if(e->ciphers)
		{
		const int *nids;
		int num_nids = e->ciphers(e, NULL, &nids, 0);
		if(num_nids > 0)
			return engine_table_register(&cipher_table,
					engine_unregister_all_ciphers, e, nids,
					num_nids, 0);
		}
	return 1;
	}

void ENGINE_register_all_ciphers()
	{
	ENGINE *e;

	for(e=ENGINE_get_first() ; e ; e=ENGINE_get_next(e))
		ENGINE_register_ciphers(e);
	}

int ENGINE_set_default_ciphers(ENGINE *e)
	{
	if(e->ciphers)
		{
		const int *nids;
		int num_nids = e->ciphers(e, NULL, &nids, 0);
		if(num_nids > 0)
			return engine_table_register(&cipher_table,
					engine_unregister_all_ciphers, e, nids,
					num_nids, 1);
		}
	return 1;
	}

/* Exposed API function to get a functional reference from the implementation
 * table (ie. try to get a functional reference from the tabled structural
 * references) for a given cipher 'nid' */
ENGINE *ENGINE_get_cipher_engine(int nid)
	{
	return engine_table_select(&cipher_table, nid);
	}

/* Obtains a cipher implementation from an ENGINE functional reference */
const EVP_CIPHER *ENGINE_get_cipher(ENGINE *e, int nid)
	{
	const EVP_CIPHER *ret;
	ENGINE_CIPHERS_PTR fn = ENGINE_get_ciphers(e);
	if(!fn || !fn(e, &ret, NULL, nid))
		{
		ENGINEerr(ENGINE_F_ENGINE_GET_CIPHER,
				ENGINE_R_UNIMPLEMENTED_CIPHER);
		return NULL;
		}
	return ret;
	}

/* Gets the cipher callback from an ENGINE structure */
ENGINE_CIPHERS_PTR ENGINE_get_ciphers(const ENGINE *e)
	{
	return e->ciphers;
	}

/* Sets the cipher callback in an ENGINE structure */
int ENGINE_set_ciphers(ENGINE *e, ENGINE_CIPHERS_PTR f)
	{
	e->ciphers = f;
	return 1;
	}