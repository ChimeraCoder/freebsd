
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
 * Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

typedef void f(int x);

static void
f_a(int i)
{
}

static void
f_b(int i)
{
}

static void
f_c(int i)
{
}

static void
f_d(int i)
{
}

static void
f_e(int i)
{
}

static void
fN(f func, int i)
{
	func(i);
}

int
main()
{
	fN(f_a, 1);
	fN(f_b, 2);
	fN(f_c, 3);
	fN(f_d, 4);
	fN(f_e, 5);
	fN(f_a, 11);
	fN(f_c, 13);
	fN(f_d, 14);
	fN(f_a, 101);
	fN(f_c, 103);
	fN(f_c, 1003);

	return (0);
}