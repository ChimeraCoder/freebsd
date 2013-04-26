
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

#include <sys/types.h>
#include <machine/cpufunc.h>
#include <machine/sysarch.h>

#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>

#include "rtld_tls.h"
#include "pthread_md.h"

struct tcb *
_tcb_ctor(struct pthread *thread, int initial)
{
	struct tcb *tcb;
	void *oldtls;

	if (initial) {
		__asm __volatile("movl %%gs:0, %0" : "=r" (oldtls));
	} else {
		oldtls = NULL;
	}

	tcb = _rtld_allocate_tls(oldtls, sizeof(struct tcb), 16);
	if (tcb) {
		tcb->tcb_thread = thread;
		tcb->tcb_spare = 0;
		bzero(&tcb->tcb_tmbx, sizeof(tcb->tcb_tmbx));
	}

	return (tcb);
}

void
_tcb_dtor(struct tcb *tcb)
{
	_rtld_free_tls(tcb, sizeof(struct tcb), 16);
}

/*
 * Initialize KSD.  This also includes setting up the LDT.
 */
struct kcb *
_kcb_ctor(struct kse *kse)
{
	struct kcb *kcb;

	kcb = malloc(sizeof(struct kcb));
	if (kcb != NULL) {
		bzero(kcb, sizeof(struct kcb));
		kcb->kcb_self = kcb;
		kcb->kcb_kse = kse;
	}
	return (kcb);
}

void
_kcb_dtor(struct kcb *kcb)
{
	free(kcb);
}

int
i386_set_gsbase(void *addr)
{

	return (sysarch(I386_SET_GSBASE, &addr));
}