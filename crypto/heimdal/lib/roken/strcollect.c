
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
#include <string.h>
#include <errno.h>
#include "roken.h"

enum { initial = 10, increment = 5 };

static char **
sub (char **argv, int i, int argc, va_list *ap)
{
    do {
	if(i == argc) {
	    /* realloc argv */
	    char **tmp = realloc(argv, (argc + increment) * sizeof(*argv));
	    if(tmp == NULL) {
		free(argv);
		errno = ENOMEM;
		return NULL;
	    }
	    argv  = tmp;
	    argc += increment;
	}
	argv[i++] = va_arg(*ap, char*);
    } while(argv[i - 1] != NULL);
    return argv;
}

/*
 * return a malloced vector of pointers to the strings in `ap'
 * terminated by NULL.
 */

ROKEN_LIB_FUNCTION char ** ROKEN_LIB_CALL
vstrcollect(va_list *ap)
{
    return sub (NULL, 0, 0, ap);
}

/*
 *
 */

ROKEN_LIB_FUNCTION char ** ROKEN_LIB_CALL
strcollect(char *first, ...)
{
    va_list ap;
    char **ret = malloc (initial * sizeof(char *));

    if (ret == NULL)
	return ret;

    ret[0] = first;
    va_start(ap, first);
    ret = sub (ret, 1, initial, &ap);
    va_end(ap);
    return ret;
}