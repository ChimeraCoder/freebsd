
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

#ifndef lint
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <errno.h>
#include <unistd.h>

#include "pwupd.h"

int
extendline(char **buf, int * buflen, int needed)
{
	if (needed > *buflen) {
		char	*tmp = realloc(*buf, needed);
		if (tmp == NULL)
			return -1;
		*buf = tmp;
		*buflen = needed;
	}
	return *buflen;
}

int
extendarray(char ***buf, int * buflen, int needed)
{
	if (needed > *buflen) {
		char	**tmp = realloc(*buf, needed * sizeof(char *));
		if (tmp == NULL)
			return -1;
		*buf = tmp;
		*buflen = needed;
	}
	return *buflen;
}