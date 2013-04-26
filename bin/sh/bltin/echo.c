
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

/*
 * Echo command.
 */

#define main echocmd

#include "bltin.h"

/* #define eflag 1 */

int
main(int argc, char *argv[])
{
	char **ap;
	char *p;
	char c;
	int count;
	int nflag = 0;
#ifndef eflag
	int eflag = 0;
#endif

	ap = argv;
	if (argc)
		ap++;
	if ((p = *ap) != NULL) {
		if (equal(p, "-n")) {
			nflag++;
			ap++;
		} else if (equal(p, "-e")) {
#ifndef eflag
			eflag++;
#endif
			ap++;
		}
	}
	while ((p = *ap++) != NULL) {
		while ((c = *p++) != '\0') {
			if (c == '\\' && eflag) {
				switch (*p++) {
				case 'a':  c = '\a';  break;
				case 'b':  c = '\b';  break;
				case 'c':  return 0;		/* exit */
				case 'e':  c = '\033';  break;
				case 'f':  c = '\f';  break;
				case 'n':  c = '\n';  break;
				case 'r':  c = '\r';  break;
				case 't':  c = '\t';  break;
				case 'v':  c = '\v';  break;
				case '\\':  break;		/* c = '\\' */
				case '0':
					c = 0;
					count = 3;
					while (--count >= 0 && (unsigned)(*p - '0') < 8)
						c = (c << 3) + (*p++ - '0');
					break;
				default:
					p--;
					break;
				}
			}
			putchar(c);
		}
		if (*ap)
			putchar(' ');
	}
	if (! nflag)
		putchar('\n');
	return 0;
}