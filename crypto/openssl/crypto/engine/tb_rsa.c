
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

/* If this symbol is defined then ENGINE_get_default_RSA(), the function that is
 * used by RSA to hook in implementation code and cache defaults (etc), will
 * display brief debugging summaries to stderr with the 'nid'. */
/* #define ENGINE_RSA_DEBUG */

static ENGINE_TABLE *rsa_table = NULL;
static const int dummy_nid = 1;

void ENGINE_unregister_RSA(ENGINE *e)
	{
	engine_table_unregister(&rsa_table, e);
	}

static void engine_unregister_all_RSA(void)
	{
	engine_table_cleanup(&rsa_table);
	}

int ENGINE_register_RSA(ENGINE *e)
	{
	if(e->rsa_meth)
		return engine_table_register(&rsa_table,
				engine_unregister_all_RSA, e, &dummy_nid, 1, 0);
	return 1;
	}

void ENGINE_register_all_RSA()
	{
	ENGINE *e;

	for(e=ENGINE_get_first() ; e ; e=ENGINE_get_next(e))
		ENGINE_register_RSA(e);
	}

int ENGINE_set_default_RSA(ENGINE *e)
	{
	if(e->rsa_meth)
		return engine_table_register(&rsa_table,
				engine_unregister_all_RSA, e, &dummy_nid, 1, 1);
	return 1;
	}

/* Exposed API function to get a functional reference from the implementation
 * table (ie. try to get a functional reference from the tabled structural
 * references). */
ENGINE *ENGINE_get_default_RSA(void)
	{
	return engine_table_select(&rsa_table, dummy_nid);
	}

/* Obtains an RSA implementation from an ENGINE functional reference */
const RSA_METHOD *ENGINE_get_RSA(const ENGINE *e)
	{
	return e->rsa_meth;
	}

/* Sets an RSA implementation in an ENGINE structure */
int ENGINE_set_RSA(ENGINE *e, const RSA_METHOD *rsa_meth)
	{
	e->rsa_meth = rsa_meth;
	return 1;
	}