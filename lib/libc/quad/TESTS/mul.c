
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
static char copyright[] =
"@(#) Copyright (c) 1992, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)mul.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <stdio.h>

main()
{
	union { long long q; unsigned long v[2]; } a, b, m;
	char buf[300];
	extern long long __muldi3(long long, long long);

	for (;;) {
		printf("> ");
		if (fgets(buf, sizeof buf, stdin) == NULL)
			break;
		if (sscanf(buf, "%lu:%lu %lu:%lu",
			    &a.v[0], &a.v[1], &b.v[0], &b.v[1]) != 4 &&
		    sscanf(buf, "0x%lx:%lx 0x%lx:%lx",
			    &a.v[0], &a.v[1], &b.v[0], &b.v[1]) != 4) {
			printf("eh?\n");
			continue;
		}
		m.q = __muldi3(a.q, b.q);
		printf("%lx:%lx * %lx:%lx => %lx:%lx\n",
		    a.v[0], a.v[1], b.v[0], b.v[1], m.v[0], m.v[1]);
		printf("  = %lX%08lX * %lX%08lX => %lX%08lX\n",
		    a.v[0], a.v[1], b.v[0], b.v[1], m.v[0], m.v[1]);
	}
	exit(0);
}