
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

/* If this symbol is defined then ENGINE_get_default_ECDSA(), the function that is
 * used by ECDSA to hook in implementation code and cache defaults (etc), will
 * display brief debugging summaries to stderr with the 'nid'. */
/* #define ENGINE_ECDSA_DEBUG */

static ENGINE_TABLE *ecdsa_table = NULL;
static const int dummy_nid = 1;

void ENGINE_unregister_ECDSA(ENGINE *e)
	{
	engine_table_unregister(&ecdsa_table, e);
	}

static void engine_unregister_all_ECDSA(void)
	{
	engine_table_cleanup(&ecdsa_table);
	}

int ENGINE_register_ECDSA(ENGINE *e)
	{
	if(e->ecdsa_meth)
		return engine_table_register(&ecdsa_table,
				engine_unregister_all_ECDSA, e, &dummy_nid, 1, 0);
	return 1;
	}

void ENGINE_register_all_ECDSA()
	{
	ENGINE *e;

	for(e=ENGINE_get_first() ; e ; e=ENGINE_get_next(e))
		ENGINE_register_ECDSA(e);
	}

int ENGINE_set_default_ECDSA(ENGINE *e)
	{
	if(e->ecdsa_meth)
		return engine_table_register(&ecdsa_table,
				engine_unregister_all_ECDSA, e, &dummy_nid, 1, 1);
	return 1;
	}

/* Exposed API function to get a functional reference from the implementation
 * table (ie. try to get a functional reference from the tabled structural
 * references). */
ENGINE *ENGINE_get_default_ECDSA(void)
	{
	return engine_table_select(&ecdsa_table, dummy_nid);
	}

/* Obtains an ECDSA implementation from an ENGINE functional reference */
const ECDSA_METHOD *ENGINE_get_ECDSA(const ENGINE *e)
	{
	return e->ecdsa_meth;
	}

/* Sets an ECDSA implementation in an ENGINE structure */
int ENGINE_set_ECDSA(ENGINE *e, const ECDSA_METHOD *ecdsa_meth)
	{
	e->ecdsa_meth = ecdsa_meth;
	return 1;
	}