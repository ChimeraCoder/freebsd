
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "extern.h"

void
c_link(const char *file1, off_t skip1, const char *file2, off_t skip2)
{
	char buf1[PATH_MAX], *p1;
	char buf2[PATH_MAX], *p2;
	int dfound, len1, len2;
	off_t byte;
	u_char ch;

	if ((len1 = readlink(file1, buf1, sizeof(buf1) - 1)) < 0) {
		if (!sflag)
			err(ERR_EXIT, "%s", file1);
		else
			exit(ERR_EXIT);
	}

	if ((len2 = readlink(file2, buf2, sizeof(buf2) - 1)) < 0) {
		if (!sflag)
			err(ERR_EXIT, "%s", file2);
		else
			exit(ERR_EXIT);
	}

	if (skip1 > len1)
		skip1 = len1;
	buf1[len1] = '\0';

	if (skip2 > len2)
		skip2 = len2;
	buf2[len2] = '\0';

	dfound = 0;
	byte = 1;
	for (p1 = buf1 + skip1, p2 = buf2 + skip2; *p1 && *p2; p1++, p2++) {
		if ((ch = *p1) != *p2) {
			if (xflag) {
				dfound = 1;
				(void)printf("%08llx %02x %02x\n",
				    (long long)byte - 1, ch, *p2);
			} else if (lflag) {
				dfound = 1;
				(void)printf("%6lld %3o %3o\n",
				    (long long)byte, ch, *p2);
			} else
				diffmsg(file1, file2, byte, 1);
				/* NOTREACHED */
		}
		byte++;
	}

	if (*p1 || *p2)
		eofmsg (*p1 ? file2 : file1);
	if (dfound)
		exit(DIFF_EXIT);
}