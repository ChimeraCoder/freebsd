
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

#include <stdint.h>
#include <stdlib.h>
#include <strings.h>

int
baz(void)
{
	return (8);
}

static int
foo(void)
{
	/*
	 * In order to assure that our helper is properly employed to identify
	 * the frame, we're going to trampoline through data.
	 */
	uint32_t instr[] = {
	    0x9de3bfa0,		/* save %sp, -0x60, %sp	*/
	    0x40000000,		/* call baz		*/
	    0x01000000,		/* nop			*/
	    0x81c7e008,		/* ret			*/
	    0x81e80000		/* restore		*/
	};
	uint32_t *fp = malloc(sizeof (instr));

	/*
	 * Do our little relocation dance.
	 */
	instr[1] |= ((uintptr_t)baz - (uintptr_t)&fp[1]) >> 2;

	/*
	 * Copy the code to the heap (it's a pain to build in ON with an
	 * executable stack).
	 */
	bcopy(instr, fp, sizeof (instr));

	(*(int (*)(void))fp)();

	free(fp);

	return (0);
}

int
main(int argc, char **argv)
{
	for (;;) {
		foo();
	}

	return (0);
}