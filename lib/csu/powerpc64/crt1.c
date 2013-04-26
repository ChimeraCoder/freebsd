
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

/* The entry function. */
/*
 * First 5 arguments are specified by the PowerPC SVR4 ABI.
 * The last argument, ps_strings, is a BSD extension.
 */
/* ARGSUSED */
void
_start(int argc, char **argv, char **env,
    const struct Struct_Obj_Entry *obj __unused, void (*cleanup)(void),
    struct ps_strings *ps_strings)
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