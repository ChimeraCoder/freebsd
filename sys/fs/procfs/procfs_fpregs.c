
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

#include "opt_compat.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/ptrace.h>
#include <sys/sysent.h>
#include <sys/uio.h>

#include <machine/reg.h>

#include <fs/pseudofs/pseudofs.h>
#include <fs/procfs/procfs.h>

#ifdef COMPAT_FREEBSD32
#include <sys/procfs.h>
#include <machine/fpu.h>

/*
 * PROC(write, fpregs, td2, &r) becomes
 * proc_write_fpregs(td2, &r)   or
 * proc_write_fpregs32(td2, &r32)
 *
 * UIOMOVE_FROMBUF(r, uio) becomes
 * uiomove_frombuf(&r, sizeof(r), uio)  or
 * uiomove_frombuf(&r32, sizeof(r32), uio)
 */
#define	PROC(d, w, t, r)	wrap32 ? \
	proc_ ## d ## _ ## w ## 32(t, r ## 32) : \
	proc_ ## d ## _ ## w(t, r)
#define	UIOMOVE_FROMBUF(k, u)	wrap32 ? \
	uiomove_frombuf(& k ## 32, sizeof(k ## 32), u) : \
	uiomove_frombuf(& k, sizeof(k), u)
#else
#define	PROC(d, w, t, r)	proc_ ## d ## _ ## w(t, r)
#define	UIOMOVE_FROMBUF(k, u)	uiomove_frombuf(& k, sizeof(k), u)
#endif

int
procfs_doprocfpregs(PFS_FILL_ARGS)
{
	int error;
	struct fpreg r;
	struct thread *td2;
#ifdef COMPAT_FREEBSD32
	struct fpreg32 r32;
	int wrap32 = 0;
#endif

	if (uio->uio_offset != 0)
		return (0);

	PROC_LOCK(p);
	KASSERT(p->p_lock > 0, ("proc not held"));
	if (p_candebug(td, p)) {
		PROC_UNLOCK(p);
		return (EPERM);
	}
	if (!P_SHOULDSTOP(p)) {
		PROC_UNLOCK(p);
		return (EBUSY);
	}

	/* XXXKSE: */
	td2 = FIRST_THREAD_IN_PROC(p);
#ifdef COMPAT_FREEBSD32
	if (SV_CURPROC_FLAG(SV_ILP32)) {
		if (SV_PROC_FLAG(td2->td_proc, SV_ILP32) == 0) {
			PROC_UNLOCK(p);
			return (EINVAL);
		}
		wrap32 = 1;
	}
#endif
	error = PROC(read, fpregs, td2, &r);
	if (error == 0) {
		PROC_UNLOCK(p);
		error = UIOMOVE_FROMBUF(r, uio);
		PROC_LOCK(p);
	}
	if (error == 0 && uio->uio_rw == UIO_WRITE) {
		if (!P_SHOULDSTOP(p))
			error = EBUSY;
		else
			/* XXXKSE: */
			error = PROC(write, fpregs, td2, &r);
	}
	PROC_UNLOCK(p);

	return (error);
}