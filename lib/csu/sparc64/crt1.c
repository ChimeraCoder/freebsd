
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

extern void __sparc_utrap_setup(void);

#ifdef GCRT
extern void _mcleanup(void);
extern void monstartup(void *, void *);
extern int eprol;
extern int etext;
#endif

void _start(char **, void (*)(void), struct Struct_Obj_Entry *,
    struct ps_strings *);

/* The entry function. */
/*
 * %o0 holds ps_strings pointer.
 *
 * Note: kernel may (is not set in stone yet) pass ELF aux vector in %o1,
 * but for now we do not use it here.
 *
 * The SPARC compliance definitions specifies that the kernel pass the
 * address of a function to be executed on exit in %g1. We do not make
 * use of it as it is quite broken, because gcc can use this register
 * as a temporary, so it is not safe from C code. Its even more broken
 * for dynamic executables since rtld runs first.
 */
/* ARGSUSED */
void
_start(char **ap, void (*cleanup)(void), struct Struct_Obj_Entry *obj __unused,
    struct ps_strings *ps_strings __unused)
{
	int argc;
	char **argv;
	char **env;

	argc = *(long *)(void *)ap;
	argv = ap + 1;
	env  = ap + 2 + argc;
	handle_argv(argc, argv, env);

	if (&_DYNAMIC != NULL)
		atexit(cleanup);
	else {
		__sparc_utrap_setup();
		_init_tls();
	}
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