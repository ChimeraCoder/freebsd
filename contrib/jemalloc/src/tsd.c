
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
/******************************************************************************/
/* Data. */
static unsigned ncleanups;
static malloc_tsd_cleanup_t cleanups[MALLOC_TSD_CLEANUPS_MAX];

/******************************************************************************/

void *
malloc_tsd_malloc(size_t size)
{

	/* Avoid choose_arena() in order to dodge bootstrapping issues. */
	return (arena_malloc(arenas[0], size, false, false));
}

void
malloc_tsd_dalloc(void *wrapper)
{

	idalloc(wrapper);
}

void
malloc_tsd_no_cleanup(void *arg)
{

	not_reached();
}

#if defined(JEMALLOC_MALLOC_THREAD_CLEANUP) || defined(_WIN32)
#ifndef _WIN32
JEMALLOC_EXPORT
#endif
void
_malloc_thread_cleanup(void)
{
	bool pending[MALLOC_TSD_CLEANUPS_MAX], again;
	unsigned i;

	for (i = 0; i < ncleanups; i++)
		pending[i] = true;

	do {
		again = false;
		for (i = 0; i < ncleanups; i++) {
			if (pending[i]) {
				pending[i] = cleanups[i]();
				if (pending[i])
					again = true;
			}
		}
	} while (again);
}
#endif

void
malloc_tsd_cleanup_register(bool (*f)(void))
{

	assert(ncleanups < MALLOC_TSD_CLEANUPS_MAX);
	cleanups[ncleanups] = f;
	ncleanups++;
}

void
malloc_tsd_boot(void)
{

	ncleanups = 0;
}

#ifdef _WIN32
static BOOL WINAPI
_tls_callback(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{

	switch (fdwReason) {
#ifdef JEMALLOC_LAZY_LOCK
	case DLL_THREAD_ATTACH:
		isthreaded = true;
		break;
#endif
	case DLL_THREAD_DETACH:
		_malloc_thread_cleanup();
		break;
	default:
		break;
	}
	return (true);
}

#ifdef _MSC_VER
#  ifdef _M_IX86
#    pragma comment(linker, "/INCLUDE:__tls_used")
#  else
#    pragma comment(linker, "/INCLUDE:_tls_used")
#  endif
#  pragma section(".CRT$XLY",long,read)
#endif
JEMALLOC_SECTION(".CRT$XLY") JEMALLOC_ATTR(used)
static const BOOL	(WINAPI *tls_callback)(HINSTANCE hinstDLL,
    DWORD fdwReason, LPVOID lpvReserved) = _tls_callback;
#endif