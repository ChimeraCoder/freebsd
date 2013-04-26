
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void grow1(int);

void
grow(int frame)
{
	/*
	 * Create a ridiculously large stack - enough to push us over
	 * the default setting of 'dtrace_ustackdepth_max' (2048).
	 */
	if (frame >= 2048)
		for (;;)
			getpid();

	grow1(++frame);
}

void
grow1(int frame)
{
	grow(++frame);
}

int
main(int argc, char *argv[])
{
	grow(1);

	return (0);
}