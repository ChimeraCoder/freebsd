
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

/* If this symbol is defined then ENGINE_get_default_DSA(), the function that is
 * used by DSA to hook in implementation code and cache defaults (etc), will
 * display brief debugging summaries to stderr with the 'nid'. */
/* #define ENGINE_DSA_DEBUG */

static ENGINE_TABLE *dsa_table = NULL;
static const int dummy_nid = 1;

void ENGINE_unregister_DSA(ENGINE *e)
	{
	engine_table_unregister(&dsa_table, e);
	}

static void engine_unregister_all_DSA(void)
	{
	engine_table_cleanup(&dsa_table);
	}

int ENGINE_register_DSA(ENGINE *e)
	{
	if(e->dsa_meth)
		return engine_table_register(&dsa_table,
				engine_unregister_all_DSA, e, &dummy_nid, 1, 0);
	return 1;
	}

void ENGINE_register_all_DSA()
	{
	ENGINE *e;

	for(e=ENGINE_get_first() ; e ; e=ENGINE_get_next(e))
		ENGINE_register_DSA(e);
	}

int ENGINE_set_default_DSA(ENGINE *e)
	{
	if(e->dsa_meth)
		return engine_table_register(&dsa_table,
				engine_unregister_all_DSA, e, &dummy_nid, 1, 1);
	return 1;
	}

/* Exposed API function to get a functional reference from the implementation
 * table (ie. try to get a functional reference from the tabled structural
 * references). */
ENGINE *ENGINE_get_default_DSA(void)
	{
	return engine_table_select(&dsa_table, dummy_nid);
	}

/* Obtains an DSA implementation from an ENGINE functional reference */
const DSA_METHOD *ENGINE_get_DSA(const ENGINE *e)
	{
	return e->dsa_meth;
	}

/* Sets an DSA implementation in an ENGINE structure */
int ENGINE_set_DSA(ENGINE *e, const DSA_METHOD *dsa_meth)
	{
	e->dsa_meth = dsa_meth;
	return 1;
	}