
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

#include <stdarg.h>
#include <stdlib.h>
#include "roken.h"

struct rk_strpool {
    char *str;
    size_t len;
};

/*
 *
 */

ROKEN_LIB_FUNCTION void ROKEN_LIB_CALL
rk_strpoolfree(struct rk_strpool *p)
{
    if (p->str) {
	free(p->str);
	p->str = NULL;
    }
    free(p);
}

/*
 *
 */

ROKEN_LIB_FUNCTION struct rk_strpool * ROKEN_LIB_CALL
rk_strpoolprintf(struct rk_strpool *p, const char *fmt, ...)
{
    va_list ap;
    char *str, *str2;
    int len;

    if (p == NULL) {
	p = malloc(sizeof(*p));
	if (p == NULL)
	    return NULL;
	p->str = NULL;
	p->len = 0;
    }
    va_start(ap, fmt);
    len = vasprintf(&str, fmt, ap);
    va_end(ap);
    if (str == NULL) {
	rk_strpoolfree(p);
	return NULL;
    }
    str2 = realloc(p->str, len + p->len + 1);
    if (str2 == NULL) {
	rk_strpoolfree(p);
	return NULL;
    }
    p->str = str2;
    memcpy(p->str + p->len, str, len + 1);
    p->len += len;
    free(str);
    return p;
}

/*
 *
 */

ROKEN_LIB_FUNCTION char * ROKEN_LIB_CALL
rk_strpoolcollect(struct rk_strpool *p)
{
    char *str;
    if (p == NULL)
	return strdup("");
    str = p->str;
    p->str = NULL;
    free(p);
    return str;
}