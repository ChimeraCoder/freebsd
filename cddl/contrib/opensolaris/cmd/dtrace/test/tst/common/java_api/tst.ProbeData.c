
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

typedef void f(char *);

static void
f_a(char *a)
{
}

static void
f_b(char *a)
{
}

static void
f_c(char *a)
{
}

static void
f_d(char *a)
{
}

static void
f_e(char *a)
{
}

static void
fN(f func, char *a, int i)
{
	func(a);
}

static void
fN2(f func, char *a, int i)
{
	func(a);
}

int
main()
{
	/*
	 * Avoid length of 1, 2, 4, or 8 bytes so DTrace will treat the data as
	 * a byte array.
	 */
	char a[] = {(char)-7, (char)201, (char)0, (char)0, (char)28, (char)1};
	char b[] = {(char)84, (char)69, (char)0, (char)0, (char)28, (char)0};
	char c[] = {(char)84, (char)69, (char)0, (char)0, (char)28, (char)1};
	char d[] = {(char)-7, (char)201, (char)0, (char)0, (char)29, (char)0};
	char e[] = {(char)84, (char)69, (char)0, (char)0, (char)28, (char)0};

	fN(f_a, a, 1);
	fN(f_b, b, 0);
	fN(f_d, d, 102);
	fN2(f_e, e, -2);
	fN(f_c, c, 0);
	fN(f_a, a, -1);
	fN(f_d, d, 101);
	fN(f_e, e, -2);
	fN(f_e, e, 2);
	fN2(f_e, e, 2);

	return (0);
}