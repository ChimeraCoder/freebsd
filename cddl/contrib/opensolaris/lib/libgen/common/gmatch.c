
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

/*
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#if defined(sun)
#pragma weak gmatch = _gmatch

#include "gen_synonyms.h"
#endif
#include <sys/types.h>
#include <libgen.h>
#include <stdlib.h>
#include <limits.h>
#if defined(sun)
#include <widec.h>
#include "_range.h"
#else
/* DOODAD */ static int multibyte = 0;
#define WCHAR_CSMASK    0x30000000
#define valid_range(c1, c2) \
    (((c1) & WCHAR_CSMASK) == ((c2) & WCHAR_CSMASK) && \
    ((c1) > 0xff || !iscntrl((int)c1)) && ((c2) > 0xff || \
    !iscntrl((int)c2)))
#endif

#define	Popwchar(p, c) \
	n = mbtowc(&cl, p, MB_LEN_MAX); \
	c = cl; \
	if (n <= 0) \
		return (0); \
	p += n

int
gmatch(const char *s, const char *p)
{
	const char	*olds;
	wchar_t		scc, c;
	int 		n;
	wchar_t		cl;

	olds = s;
	n = mbtowc(&cl, s, MB_LEN_MAX);
	if (n <= 0) {
		s++;
		scc = n;
	} else {
		scc = cl;
		s += n;
	}
	n = mbtowc(&cl, p, MB_LEN_MAX);
	if (n < 0)
		return (0);
	if (n == 0)
		return (scc == 0);
	p += n;
	c = cl;

	switch (c) {
	case '[':
		if (scc <= 0)
			return (0);
	{
			int ok;
			wchar_t lc = 0;
			int notflag = 0;

			ok = 0;
			if (*p == '!') {
				notflag = 1;
				p++;
			}
			Popwchar(p, c);
			do
			{
				if (c == '-' && lc && *p != ']') {
					Popwchar(p, c);
					if (c == '\\') {
						Popwchar(p, c);
					}
					if (notflag) {
						if (!multibyte ||
						    valid_range(lc, c)) {
							if (scc < lc || scc > c)
								ok++;
							else
								return (0);
						}
					} else {
						if (!multibyte ||
						    valid_range(lc, c))
							if (lc <= scc &&
							    scc <= c)
								ok++;
					}
				} else if (c == '\\') {
					/* skip to quoted character */
					Popwchar(p, c);
				}
				lc = c;
				if (notflag) {
					if (scc != lc)
						ok++;
					else
						return (0);
				}
				else
				{
					if (scc == lc)
						ok++;
				}
				Popwchar(p, c);
			} while (c != ']');
			return (ok ? gmatch(s, p) : 0);
		}

	case '\\':
		/* skip to quoted character and see if it matches */
		Popwchar(p, c);

	default:
		if (c != scc)
			return (0);
			/*FALLTHRU*/

	case '?':
		return (scc > 0 ? gmatch(s, p) : 0);

	case '*':
		while (*p == '*')
			p++;

		if (*p == 0)
			return (1);
		s = olds;
		while (*s) {
			if (gmatch(s, p))
				return (1);
			n = mbtowc(&cl, s, MB_LEN_MAX);
			if (n < 0)
				/* skip past illegal byte sequence */
				s++;
			else
				s += n;
		}
		return (0);
	}
}