
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

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/syscallsubr.h>
#include <sys/sysent.h>
#include <sys/systm.h>
#include <sys/sysproto.h>
#include <sys/signalvar.h>
#include <sys/ucontext.h>

/*
 * The first two fields of a ucontext_t are the signal mask and the machine
 * context.  The next field is uc_link; we want to avoid destroying the link
 * when copying out contexts.
 */
#define	UC_COPY_SIZE	offsetof(ucontext_t, uc_link)

#ifndef _SYS_SYSPROTO_H_
struct getcontext_args {
	struct __ucontext *ucp;
}
struct setcontext_args {
	const struct __ucontext_t *ucp;
}
struct swapcontext_args {
	struct __ucontext *oucp;
	const struct __ucontext_t *ucp;
}
#endif

int
sys_getcontext(struct thread *td, struct getcontext_args *uap)
{
	ucontext_t uc;
	int ret;

	if (uap->ucp == NULL)
		ret = EINVAL;
	else {
		get_mcontext(td, &uc.uc_mcontext, GET_MC_CLEAR_RET);
		PROC_LOCK(td->td_proc);
		uc.uc_sigmask = td->td_sigmask;
		PROC_UNLOCK(td->td_proc);
		bzero(uc.__spare__, sizeof(uc.__spare__));
		ret = copyout(&uc, uap->ucp, UC_COPY_SIZE);
	}
	return (ret);
}

int
sys_setcontext(struct thread *td, struct setcontext_args *uap)
{
	ucontext_t uc;
	int ret;	

	if (uap->ucp == NULL)
		ret = EINVAL;
	else {
		ret = copyin(uap->ucp, &uc, UC_COPY_SIZE);
		if (ret == 0) {
			ret = set_mcontext(td, &uc.uc_mcontext);
			if (ret == 0) {
				kern_sigprocmask(td, SIG_SETMASK, &uc.uc_sigmask,
				    NULL, 0);
			}
		}
	}
	return (ret == 0 ? EJUSTRETURN : ret);
}

int
sys_swapcontext(struct thread *td, struct swapcontext_args *uap)
{
	ucontext_t uc;
	int ret;	

	if (uap->oucp == NULL || uap->ucp == NULL)
		ret = EINVAL;
	else {
		get_mcontext(td, &uc.uc_mcontext, GET_MC_CLEAR_RET);
		bzero(uc.__spare__, sizeof(uc.__spare__));
		PROC_LOCK(td->td_proc);
		uc.uc_sigmask = td->td_sigmask;
		PROC_UNLOCK(td->td_proc);
		ret = copyout(&uc, uap->oucp, UC_COPY_SIZE);
		if (ret == 0) {
			ret = copyin(uap->ucp, &uc, UC_COPY_SIZE);
			if (ret == 0) {
				ret = set_mcontext(td, &uc.uc_mcontext);
				if (ret == 0) {
					kern_sigprocmask(td, SIG_SETMASK,
					    &uc.uc_sigmask, NULL, 0);
				}
			}
		}
	}
	return (ret == 0 ? EJUSTRETURN : ret);
}