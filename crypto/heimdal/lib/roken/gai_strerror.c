
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

#include <config.h>

#include "roken.h"

static struct gai_error {
    int code;
    const char *str;
} errors[] = {
{EAI_NOERROR,		"no error"},
#ifdef EAI_ADDRFAMILY
{EAI_ADDRFAMILY,	"address family for nodename not supported"},
#endif
{EAI_AGAIN,		"temporary failure in name resolution"},
{EAI_BADFLAGS,		"invalid value for ai_flags"},
{EAI_FAIL,		"non-recoverable failure in name resolution"},
{EAI_FAMILY,		"ai_family not supported"},
{EAI_MEMORY,		"memory allocation failure"},
#ifdef EAI_NODATA
{EAI_NODATA,		"no address associated with nodename"},
#endif
{EAI_NONAME,		"nodename nor servname provided, or not known"},
{EAI_SERVICE,		"servname not supported for ai_socktype"},
{EAI_SOCKTYPE,		"ai_socktype not supported"},
{EAI_SYSTEM,		"system error returned in errno"},
{0,			NULL},
};

/*
 *
 */

ROKEN_LIB_FUNCTION const char * ROKEN_LIB_CALL
gai_strerror(int ecode)
{
    struct gai_error *g;

    for (g = errors; g->str != NULL; ++g)
	if (g->code == ecode)
	    return g->str;
    return "unknown error code in gai_strerror";
}