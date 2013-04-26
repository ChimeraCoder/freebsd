
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
static const char rcsid[] _U_ =
    "@(#) $Header: /tcpdump/master/libpcap/etherent.c,v 1.23 2006-10-04 18:09:22 guy Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef WIN32
#include <pcap-stdinc.h>
#else /* WIN32 */
#if HAVE_INTTYPES_H
#include <inttypes.h>
#elif HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_SYS_BITYPES_H
#include <sys/bitypes.h>
#endif
#include <sys/types.h>
#endif /* WIN32 */

#include <ctype.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>

#include "pcap-int.h"

#include <pcap/namedb.h>

#ifdef HAVE_OS_PROTO_H
#include "os-proto.h"
#endif

static inline int xdtoi(int);
static inline int skip_space(FILE *);
static inline int skip_line(FILE *);

/* Hex digit to integer. */
static inline int
xdtoi(c)
	register int c;
{
	if (isdigit(c))
		return c - '0';
	else if (islower(c))
		return c - 'a' + 10;
	else
		return c - 'A' + 10;
}

static inline int
skip_space(f)
	FILE *f;
{
	int c;

	do {
		c = getc(f);
	} while (isspace(c) && c != '\n');

	return c;
}

static inline int
skip_line(f)
	FILE *f;
{
	int c;

	do
		c = getc(f);
	while (c != '\n' && c != EOF);

	return c;
}

struct pcap_etherent *
pcap_next_etherent(FILE *fp)
{
	register int c, d, i;
	char *bp;
	static struct pcap_etherent e;

	memset((char *)&e, 0, sizeof(e));
	do {
		/* Find addr */
		c = skip_space(fp);
		if (c == '\n')
			continue;

		/* If this is a comment, or first thing on line
		   cannot be etehrnet address, skip the line. */
		if (!isxdigit(c)) {
			c = skip_line(fp);
			continue;
		}

		/* must be the start of an address */
		for (i = 0; i < 6; i += 1) {
			d = xdtoi(c);
			c = getc(fp);
			if (isxdigit(c)) {
				d <<= 4;
				d |= xdtoi(c);
				c = getc(fp);
			}
			e.addr[i] = d;
			if (c != ':')
				break;
			c = getc(fp);
		}
		if (c == EOF)
			break;

		/* Must be whitespace */
		if (!isspace(c)) {
			c = skip_line(fp);
			continue;
		}
		c = skip_space(fp);

		/* hit end of line... */
		if (c == '\n')
			continue;

		if (c == '#') {
			c = skip_line(fp);
			continue;
		}

		/* pick up name */
		bp = e.name;
		/* Use 'd' to prevent buffer overflow. */
		d = sizeof(e.name) - 1;
		do {
			*bp++ = c;
			c = getc(fp);
		} while (!isspace(c) && c != EOF && --d > 0);
		*bp = '\0';

		/* Eat trailing junk */
		if (c != '\n')
			(void)skip_line(fp);

		return &e;

	} while (c != EOF);

	return (NULL);
}