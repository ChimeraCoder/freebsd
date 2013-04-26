
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <stand.h>
#include "libofw.h"

void startup(void *, int, int (*)(void *), char *, int);

__asm("				\n\
	.data			\n\
	.align 4		\n\
stack:				\n\
	.space	16388		\n\
				\n\
	.text			\n\
	.globl	_start		\n\
_start:				\n\
	lis	%r1,stack@ha	\n\
	addi	%r1,%r1,stack@l	\n\
	addi	%r1,%r1,8192	\n\
				\n\
	/* Clear the .bss!!! */	\n\
	li      %r0,0		\n\
	lis     %r8,_edata@ha	\n\
	addi    %r8,%r8,_edata@l\n\
	lis     %r9,_end@ha	\n\
	addi    %r9,%r9,_end@l	\n\
				\n\
1:	cmpw    0,%r8,%r9	\n\
	bge     2f		\n\
	stw     %r0,0(%r8)	\n\
	addi    %r8,%r8,4	\n\
	b       1b		\n\
				\n\
2:	b	startup		\n\
");

void
startup(void *vpd, int res, int (*openfirm)(void *), char *arg, int argl)
{
	main(openfirm);
}