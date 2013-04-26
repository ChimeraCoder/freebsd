
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
#include <libutil.h>
#include <stdio.h>

void
hexdump(const void *ptr, int length, const char *hdr, int flags)
{
	int i, j, k;
	int cols;
	const unsigned char *cp;
	char delim;

	if ((flags & HD_DELIM_MASK) != 0)
		delim = (flags & HD_DELIM_MASK) >> 8;
	else
		delim = ' ';

	if ((flags & HD_COLUMN_MASK) != 0)
		cols = flags & HD_COLUMN_MASK;
	else
		cols = 16;

	cp = ptr;
	for (i = 0; i < length; i+= cols) {
		if (hdr != NULL)
			printf("%s", hdr);

		if ((flags & HD_OMIT_COUNT) == 0)
			printf("%04x  ", i);

		if ((flags & HD_OMIT_HEX) == 0) {
			for (j = 0; j < cols; j++) {
				k = i + j;
				if (k < length)
					printf("%c%02x", delim, cp[k]);
				else
					printf("   ");
			}
		}

		if ((flags & HD_OMIT_CHARS) == 0) {
			printf("  |");
			for (j = 0; j < cols; j++) {
				k = i + j;
				if (k >= length)
					printf(" ");
				else if (cp[k] >= ' ' && cp[k] <= '~')
					printf("%c", cp[k]);
				else
					printf(".");
			}
			printf("|");
		}
		printf("\n");
	}
}