
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

/*
 * Linkage to services provided by the dynamic linker.
 */
#include <sys/mman.h>
#include <dlfcn.h>
#include <link.h>
#include <stddef.h>
#include "namespace.h"
#include <pthread.h>
#include "un-namespace.h"
#include "libc_private.h"

static char sorry[] = "Service unavailable";

/*
 * For ELF, the dynamic linker directly resolves references to its
 * services to functions inside the dynamic linker itself.  These
 * weak-symbol stubs are necessary so that "ld" won't complain about
 * undefined symbols.  The stubs are executed only when the program is
 * linked statically, or when a given service isn't implemented in the
 * dynamic linker.  They must return an error if called, and they must
 * be weak symbols so that the dynamic linker can override them.
 */

#pragma weak _rtld_error
void
_rtld_error(const char *fmt, ...)
{
}

#pragma weak dladdr
int
dladdr(const void *addr, Dl_info *dlip)
{
	_rtld_error(sorry);
	return 0;
}

#pragma weak dlclose
int
dlclose(void *handle)
{
	_rtld_error(sorry);
	return -1;
}

#pragma weak dlerror
char *
dlerror(void)
{
	return sorry;
}

#pragma weak dllockinit
void
dllockinit(void *context,
	   void *(*lock_create)(void *context),
	   void (*rlock_acquire)(void *lock),
	   void (*wlock_acquire)(void *lock),
	   void (*lock_release)(void *lock),
	   void (*lock_destroy)(void *lock),
	   void (*context_destroy)(void *context))
{
	if (context_destroy != NULL)
		context_destroy(context);
}

#pragma weak dlopen
void *
dlopen(const char *name, int mode)
{
	_rtld_error(sorry);
	return NULL;
}

#pragma weak dlsym
void *
dlsym(void * __restrict handle, const char * __restrict name)
{
	_rtld_error(sorry);
	return NULL;
}

#pragma weak dlfunc
dlfunc_t
dlfunc(void * __restrict handle, const char * __restrict name)
{
	_rtld_error(sorry);
	return NULL;
}

#pragma weak dlvsym
void *
dlvsym(void * __restrict handle, const char * __restrict name,
    const char * __restrict version)
{
	_rtld_error(sorry);
	return NULL;
}

#pragma weak dlinfo
int
dlinfo(void * __restrict handle, int request, void * __restrict p)
{
	_rtld_error(sorry);
	return 0;
}

#pragma weak _rtld_thread_init
void
_rtld_thread_init(void * li)
{
	_rtld_error(sorry);
}

static pthread_once_t dl_phdr_info_once = PTHREAD_ONCE_INIT;
static struct dl_phdr_info phdr_info;

static void
dl_init_phdr_info(void)
{
	Elf_Auxinfo *auxp;
	size_t phent;
	unsigned int i;

	phent = 0;
	for (auxp = __elf_aux_vector; auxp->a_type != AT_NULL; auxp++) {
		switch (auxp->a_type) {
		case AT_BASE:
			phdr_info.dlpi_addr = (Elf_Addr)auxp->a_un.a_ptr;
			break;
		case AT_EXECPATH:
			phdr_info.dlpi_name = (const char *)auxp->a_un.a_ptr;
			break;
		case AT_PHDR:
			phdr_info.dlpi_phdr =
			    (const Elf_Phdr *)auxp->a_un.a_ptr;
			break;
		case AT_PHENT:
			phent = auxp->a_un.a_val;
			break;
		case AT_PHNUM:
			phdr_info.dlpi_phnum = (Elf_Half)auxp->a_un.a_val;
			break;
		}
	}
	for (i = 0; i < phdr_info.dlpi_phnum; i++) {
		if (phdr_info.dlpi_phdr[i].p_type == PT_TLS) {
			phdr_info.dlpi_tls_modid = 1;
			phdr_info.dlpi_tls_data =
			    (void*)phdr_info.dlpi_phdr[i].p_vaddr;
		}
	}
	phdr_info.dlpi_adds = 1;
}

#pragma weak dl_iterate_phdr
int
dl_iterate_phdr(int (*callback)(struct dl_phdr_info *, size_t, void *),
    void *data)
{

	__init_elf_aux_vector();
	if (__elf_aux_vector == NULL)
		return (1);
	_once(&dl_phdr_info_once, dl_init_phdr_info);
	return (callback(&phdr_info, sizeof(phdr_info), data));
}

#pragma weak fdlopen
void *
fdlopen(int fd, int mode)
{

	_rtld_error(sorry);
	return NULL;
}

#pragma weak _rtld_atfork_pre
void
_rtld_atfork_pre(int *locks)
{
}

#pragma weak _rtld_atfork_post
void
_rtld_atfork_post(int *locks)
{
}

#pragma weak _rtld_addr_phdr
int
_rtld_addr_phdr(const void *addr, struct dl_phdr_info *phdr_info)
{

	return (0);
}

#pragma weak _rtld_get_stack_prot
int
_rtld_get_stack_prot(void)
{

	return (PROT_EXEC | PROT_READ | PROT_WRITE);
}