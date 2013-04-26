
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

#include "notes.h"

extern int main(int, char **, char **);

extern void (*__preinit_array_start[])(int, char **, char **) __hidden;
extern void (*__preinit_array_end[])(int, char **, char **) __hidden;
extern void (*__init_array_start[])(int, char **, char **) __hidden;
extern void (*__init_array_end[])(int, char **, char **) __hidden;
extern void (*__fini_array_start[])(void) __hidden;
extern void (*__fini_array_end[])(void) __hidden;
extern void _fini(void) __hidden;
extern void _init(void) __hidden;

extern int _DYNAMIC;
#pragma weak _DYNAMIC

char **environ;
const char *__progname = "";

static void
finalizer(void)
{
	void (*fn)(void);
	size_t array_size, n;

	array_size = __fini_array_end - __fini_array_start;
	for (n = array_size; n > 0; n--) {
		fn = __fini_array_start[n - 1];
		if ((uintptr_t)fn != 0 && (uintptr_t)fn != 1)
			(fn)();
	}
	_fini();
}

static inline void
handle_static_init(int argc, char **argv, char **env)
{
	void (*fn)(int, char **, char **);
	size_t array_size, n;

	if (&_DYNAMIC != NULL)
		return;

	atexit(finalizer);

	array_size = __preinit_array_end - __preinit_array_start;
	for (n = 0; n < array_size; n++) {
		fn = __preinit_array_start[n];
		if ((uintptr_t)fn != 0 && (uintptr_t)fn != 1)
			fn(argc, argv, env);
	}
	_init();
	array_size = __init_array_end - __init_array_start;
	for (n = 0; n < array_size; n++) {
		fn = __init_array_start[n];
		if ((uintptr_t)fn != 0 && (uintptr_t)fn != 1)
			fn(argc, argv, env);
	}
}

static inline void
handle_argv(int argc, char *argv[], char **env)
{
	const char *s;

	if (environ == NULL)
		environ = env;
	if (argc > 0 && argv[0] != NULL) {
		__progname = argv[0];
		for (s = __progname; *s != '\0'; s++) {
			if (*s == '/')
				__progname = s + 1;
		}
	}
}

static const struct {
	int32_t	namesz;
	int32_t	descsz;
	int32_t	type;
	char	name[sizeof(NOTE_FREEBSD_VENDOR)];
	uint32_t desc;
} crt_noinit_tag __attribute__ ((section (NOTE_SECTION),
    aligned(4))) __used = {
	.namesz = sizeof(NOTE_FREEBSD_VENDOR),
	.descsz = sizeof(uint32_t),
	.type = CRT_NOINIT_NOTETYPE,
	.name = NOTE_FREEBSD_VENDOR,
	.desc = 0
};