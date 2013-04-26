
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

#ifndef lint
#ifndef __GNUC__
#error "GCC is needed to compile this file"
#endif
#endif /* lint */

#include <stdlib.h>

#include "libc_private.h"
#include "crtbrand.c"
#include "ignore_init.c"

struct Struct_Obj_Entry;
struct ps_strings;

extern void _start(int, char **, char **, const struct Struct_Obj_Entry *,
    void (*)(void), struct ps_strings *);

#ifdef GCRT
extern void _mcleanup(void);
extern void monstartup(void *, void *);
extern int eprol;
extern int etext;
#endif

struct ps_strings *__ps_strings;

void __start(int, char **, char **, struct ps_strings *,
    const struct Struct_Obj_Entry *, void (*)(void));

/* The entry function. */
__asm("	.text			\n"
"	.align	0		\n"
"	.globl	_start		\n"
"	_start:			\n"
"	mov	r5, r2		/* cleanup */		\n"
"	mov	r4, r1		/* obj_main */		\n"
"	mov	r3, r0		/* ps_strings */	\n"
"	/* Get argc, argv, and envp from stack */	\n"
"	ldr	r0, [sp, #0x0000]	\n"
"	add	r1, sp, #0x0004		\n"
"	add	r2, r1, r0, lsl #2	\n"
"	add	r2, r2, #0x0004		\n"
"	/* Ensure the stack is properly aligned before calling C code. */\n"
"	bic	sp, sp, #7	\n"
"	sub	sp, sp, #8	\n"
"	str	r5, [sp, #4]	\n"
"	str	r4, [sp, #0]	\n"
"\n"
"	b	 __start  ");
/* ARGSUSED */
void
__start(int argc, char **argv, char **env, struct ps_strings *ps_strings,
    const struct Struct_Obj_Entry *obj __unused, void (*cleanup)(void))
{

	handle_argv(argc, argv, env);

	if (ps_strings != (struct ps_strings *)0)
		__ps_strings = ps_strings;

	if (&_DYNAMIC != NULL)
		atexit(cleanup);
	else
		_init_tls();
#ifdef GCRT
	atexit(_mcleanup);
	monstartup(&eprol, &etext);
#endif
	handle_static_init(argc, argv, env);
	exit(main(argc, argv, env));
}

#ifdef GCRT
__asm__(".text");
__asm__("eprol:");
__asm__(".previous");
#endif