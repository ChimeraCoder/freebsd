
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

#include <stdlib.h>
#include <err.h>

#include "roken.h"

/*
 * Like realloc but never fails.
 */

ROKEN_LIB_FUNCTION void * ROKEN_LIB_CALL
erealloc (void *ptr, size_t sz)
{
    void *tmp = realloc (ptr, sz);

    if (tmp == NULL && sz != 0)
	errx (1, "realloc %lu failed", (unsigned long)sz);
    return tmp;
}