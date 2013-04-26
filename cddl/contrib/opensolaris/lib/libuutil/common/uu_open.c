
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

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include "libuutil_common.h"

#include <sys/time.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

#ifdef _LP64
#define	TMPPATHFMT	"%s/uu%ld"
#else /* _LP64 */
#define	TMPPATHFMT	"%s/uu%lld"
#endif /* _LP64 */

/*ARGSUSED*/
int
uu_open_tmp(const char *dir, uint_t uflags)
{
	int f;
	char *fname = uu_zalloc(PATH_MAX);

	if (fname == NULL)
		return (-1);

	for (;;) {
		(void) snprintf(fname, PATH_MAX, "%s/uu%lld", dir, gethrtime());

		f = open(fname, O_CREAT | O_EXCL | O_RDWR, 0600);

		if (f >= 0 || errno != EEXIST)
			break;
	}

	if (f >= 0)
		(void) unlink(fname);

	uu_free(fname);

	return (f);
}