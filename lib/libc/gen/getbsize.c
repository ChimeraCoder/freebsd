
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getbsize.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *
getbsize(headerlenp, blocksizep)
	int *headerlenp;
	long *blocksizep;
{
	static char header[20];
	long n, max, mul, blocksize;
	char *ep, *p;
	const char *form;

#define	KB	(1024L)
#define	MB	(1024L * 1024L)
#define	GB	(1024L * 1024L * 1024L)
#define	MAXB	GB		/* No tera, peta, nor exa. */
	form = "";
	if ((p = getenv("BLOCKSIZE")) != NULL && *p != '\0') {
		if ((n = strtol(p, &ep, 10)) < 0)
			goto underflow;
		if (n == 0)
			n = 1;
		if (*ep && ep[1])
			goto fmterr;
		switch (*ep) {
		case 'G': case 'g':
			form = "G";
			max = MAXB / GB;
			mul = GB;
			break;
		case 'K': case 'k':
			form = "K";
			max = MAXB / KB;
			mul = KB;
			break;
		case 'M': case 'm':
			form = "M";
			max = MAXB / MB;
			mul = MB;
			break;
		case '\0':
			max = MAXB;
			mul = 1;
			break;
		default:
fmterr:			warnx("%s: unknown blocksize", p);
			n = 512;
			max = MAXB;
			mul = 1;
			break;
		}
		if (n > max) {
			warnx("maximum blocksize is %ldG", MAXB / GB);
			n = max;
		}
		if ((blocksize = n * mul) < 512) {
underflow:		warnx("minimum blocksize is 512");
			form = "";
			blocksize = n = 512;
		}
	} else
		blocksize = n = 512;

	(void)snprintf(header, sizeof(header), "%ld%s-blocks", n, form);
	*headerlenp = strlen(header);
	*blocksizep = blocksize;
	return (header);
}