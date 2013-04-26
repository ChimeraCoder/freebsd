
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

#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>

#include <machine/sysarch.h>

#include "pthread_md.h"

struct arm_tp **arm_tp = (struct arm_tp **)ARM_TP_ADDRESS;

struct tcb *
_tcb_ctor(struct pthread *thread, int initial)
{
	struct tcb *tcb;

	if ((tcb = malloc(sizeof(struct tcb)))) {
		bzero(tcb, sizeof(struct tcb));
		tcb->tcb_thread = thread;
		/* XXX - Allocate tdv/tls */
	}
	return (tcb);
}

void
_tcb_dtor(struct tcb *tcb)
{

	free(tcb);
}

struct kcb *
_kcb_ctor(struct kse *kse)
{
	struct kcb *kcb;

	kcb = malloc(sizeof(struct kcb));
	if (kcb != NULL) {
		bzero(kcb, sizeof(struct kcb));
		kcb->kcb_faketcb.tcb_isfake = 1;
		kcb->kcb_faketcb.tcb_tmbx.tm_flags = TMF_NOUPCALL;
		kcb->kcb_curtcb = &kcb->kcb_faketcb;
		kcb->kcb_kse = kse;
	}
	return (kcb);
}

void
_kcb_dtor(struct kcb *kcb)
{
	free(kcb);
}