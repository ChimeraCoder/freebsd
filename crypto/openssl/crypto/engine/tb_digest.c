
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

/* If this symbol is defined then ENGINE_get_digest_engine(), the function that
 * is used by EVP to hook in digest code and cache defaults (etc), will display
 * brief debugging summaries to stderr with the 'nid'. */
/* #define ENGINE_DIGEST_DEBUG */

static ENGINE_TABLE *digest_table = NULL;

void ENGINE_unregister_digests(ENGINE *e)
	{
	engine_table_unregister(&digest_table, e);
	}

static void engine_unregister_all_digests(void)
	{
	engine_table_cleanup(&digest_table);
	}

int ENGINE_register_digests(ENGINE *e)
	{
	if(e->digests)
		{
		const int *nids;
		int num_nids = e->digests(e, NULL, &nids, 0);
		if(num_nids > 0)
			return engine_table_register(&digest_table,
					engine_unregister_all_digests, e, nids,
					num_nids, 0);
		}
	return 1;
	}

void ENGINE_register_all_digests()
	{
	ENGINE *e;

	for(e=ENGINE_get_first() ; e ; e=ENGINE_get_next(e))
		ENGINE_register_digests(e);
	}

int ENGINE_set_default_digests(ENGINE *e)
	{
	if(e->digests)
		{
		const int *nids;
		int num_nids = e->digests(e, NULL, &nids, 0);
		if(num_nids > 0)
			return engine_table_register(&digest_table,
					engine_unregister_all_digests, e, nids,
					num_nids, 1);
		}
	return 1;
	}

/* Exposed API function to get a functional reference from the implementation
 * table (ie. try to get a functional reference from the tabled structural
 * references) for a given digest 'nid' */
ENGINE *ENGINE_get_digest_engine(int nid)
	{
	return engine_table_select(&digest_table, nid);
	}

/* Obtains a digest implementation from an ENGINE functional reference */
const EVP_MD *ENGINE_get_digest(ENGINE *e, int nid)
	{
	const EVP_MD *ret;
	ENGINE_DIGESTS_PTR fn = ENGINE_get_digests(e);
	if(!fn || !fn(e, &ret, NULL, nid))
		{
		ENGINEerr(ENGINE_F_ENGINE_GET_DIGEST,
				ENGINE_R_UNIMPLEMENTED_DIGEST);
		return NULL;
		}
	return ret;
	}

/* Gets the digest callback from an ENGINE structure */
ENGINE_DIGESTS_PTR ENGINE_get_digests(const ENGINE *e)
	{
	return e->digests;
	}

/* Sets the digest callback in an ENGINE structure */
int ENGINE_set_digests(ENGINE *e, ENGINE_DIGESTS_PTR f)
	{
	e->digests = f;
	return 1;
	}