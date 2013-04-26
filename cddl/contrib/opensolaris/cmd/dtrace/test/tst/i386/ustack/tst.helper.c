
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
	uint8_t instr[] = {
	    0x55,			/* pushl %ebp		*/
	    0x8b, 0xec,			/* movl  %esp, %ebp	*/
	    0xe8, 0x0, 0x0, 0x0, 0x0,	/* call  baz		*/
	    0x8b, 0xe5,			/* movl  %ebp, %esp	*/
	    0x5d,			/* popl  %ebp		*/
	    0xc3			/* ret			*/
	};
	uint8_t *fp = malloc(sizeof (instr));

	/*
	 * Do our little relocation dance.
	 */
	*((int *)&instr[4]) = (uintptr_t)baz - (uintptr_t)&fp[8];

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