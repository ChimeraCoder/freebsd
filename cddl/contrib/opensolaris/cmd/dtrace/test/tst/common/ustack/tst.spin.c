
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
 * Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <unistd.h>

volatile long long count = 0;

int
baz(int a)
{
	(void) getpid();
	while (count != -1) {
		count++;
		a++;
	}

	return (a + 1);
}

int
bar(int a)
{
	return (baz(a + 1) - 1);
}

int
foo(int a, int b)
{
	return (bar(a) - b);
}

int
main(int argc, char **argv)
{
	return (foo(argc, (int)argv) == 0);
}