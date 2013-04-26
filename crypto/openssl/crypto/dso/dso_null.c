
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

/* This "NULL" method is provided as the fallback for systems that have
 * no appropriate support for "shared-libraries". */

#include <stdio.h>
#include "cryptlib.h"
#include <openssl/dso.h>

static DSO_METHOD dso_meth_null = {
	"NULL shared library method",
	NULL, /* load */
	NULL, /* unload */
	NULL, /* bind_var */
	NULL, /* bind_func */
/* For now, "unbind" doesn't exist */
#if 0
	NULL, /* unbind_var */
	NULL, /* unbind_func */
#endif
	NULL, /* ctrl */
	NULL, /* dso_name_converter */
	NULL, /* dso_merger */
	NULL, /* init */
	NULL, /* finish */
	NULL, /* pathbyaddr */
	NULL  /* globallookup */
	};

DSO_METHOD *DSO_METHOD_null(void)
	{
	return(&dso_meth_null);
	}