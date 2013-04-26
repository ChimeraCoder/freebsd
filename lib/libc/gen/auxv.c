
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

#include "namespace.h"
#include <elf.h>
#include <errno.h>
#include <link.h>
#include <pthread.h>
#include <string.h>
#include "un-namespace.h"
#include "libc_private.h"

extern char **environ;
extern int _DYNAMIC;
#pragma weak _DYNAMIC

void *__elf_aux_vector;
static pthread_once_t aux_vector_once = PTHREAD_ONCE_INIT;

static void
init_aux_vector_once(void)
{
	Elf_Addr *sp;

	sp = (Elf_Addr *)environ;
	while (*sp++ != 0)
		;
	__elf_aux_vector = (Elf_Auxinfo *)sp;
}

void
__init_elf_aux_vector(void)
{

	if (&_DYNAMIC != NULL)
		return;
	_once(&aux_vector_once, init_aux_vector_once);
}

static pthread_once_t aux_once = PTHREAD_ONCE_INIT;
static int pagesize, osreldate, canary_len, ncpus, pagesizes_len;
static char *canary, *pagesizes;
static void *timekeep;

static void
init_aux(void)
{
	Elf_Auxinfo *aux;

	for (aux = __elf_aux_vector; aux->a_type != AT_NULL; aux++) {
		switch (aux->a_type) {
		case AT_CANARY:
			canary = (char *)(aux->a_un.a_ptr);
			break;

		case AT_CANARYLEN:
			canary_len = aux->a_un.a_val;
			break;

		case AT_PAGESIZES:
			pagesizes = (char *)(aux->a_un.a_ptr);
			break;

		case AT_PAGESIZESLEN:
			pagesizes_len = aux->a_un.a_val;
			break;

		case AT_PAGESZ:
			pagesize = aux->a_un.a_val;
			break;

		case AT_OSRELDATE:
			osreldate = aux->a_un.a_val;
			break;

		case AT_NCPUS:
			ncpus = aux->a_un.a_val;
			break;

		case AT_TIMEKEEP:
			timekeep = aux->a_un.a_ptr;
			break;
		}
	}
}

int
_elf_aux_info(int aux, void *buf, int buflen)
{
	int res;

	__init_elf_aux_vector();
	if (__elf_aux_vector == NULL)
		return (ENOSYS);
	_once(&aux_once, init_aux);

	switch (aux) {
	case AT_CANARY:
		if (canary != NULL && canary_len >= buflen) {
			memcpy(buf, canary, buflen);
			memset(canary, 0, canary_len);
			canary = NULL;
			res = 0;
		} else
			res = ENOENT;
		break;
	case AT_PAGESIZES:
		if (pagesizes != NULL && pagesizes_len >= buflen) {
			memcpy(buf, pagesizes, buflen);
			res = 0;
		} else
			res = ENOENT;
		break;

	case AT_PAGESZ:
		if (buflen == sizeof(int)) {
			if (pagesize != 0) {
				*(int *)buf = pagesize;
				res = 0;
			} else
				res = ENOENT;
		} else
			res = EINVAL;
		break;
	case AT_OSRELDATE:
		if (buflen == sizeof(int)) {
			if (osreldate != 0) {
				*(int *)buf = osreldate;
				res = 0;
			} else
				res = ENOENT;
		} else
			res = EINVAL;
		break;
	case AT_NCPUS:
		if (buflen == sizeof(int)) {
			if (ncpus != 0) {
				*(int *)buf = ncpus;
				res = 0;
			} else
				res = ENOENT;
		} else
			res = EINVAL;
		break;
	case AT_TIMEKEEP:
		if (buflen == sizeof(void *)) {
			if (timekeep != NULL) {
				*(void **)buf = timekeep;
				res = 0;
			} else
				res = ENOENT;
		} else
			res = EINVAL;
		break;
	default:
		res = ENOENT;
		break;
	}
	return (res);
}