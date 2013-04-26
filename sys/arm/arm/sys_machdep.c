
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

#include "opt_capsicum.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/capability.h>
#include <sys/proc.h>
#include <sys/sysproto.h>
#include <sys/syscall.h>
#include <sys/sysent.h>

#include <machine/sysarch.h>

#ifndef _SYS_SYSPROTO_H_
struct sysarch_args {
	int op;
	char *parms;
};
#endif

/* Prototypes */
static int arm32_sync_icache (struct thread *, void *);
static int arm32_drain_writebuf(struct thread *, void *);

static int
arm32_sync_icache(struct thread *td, void *args)
{
	struct arm_sync_icache_args ua;
	int error;

	if ((error = copyin(args, &ua, sizeof(ua))) != 0)
		return (error);

	cpu_icache_sync_range(ua.addr, ua.len);

	td->td_retval[0] = 0;
	return (0);
}

static int
arm32_drain_writebuf(struct thread *td, void *args)
{
	/* No args. */

	td->td_retval[0] = 0;
	cpu_drain_writebuf();
	return (0);
}

static int
arm32_set_tp(struct thread *td, void *args)
{

	if (td != curthread)
		td->td_md.md_tp = (register_t)args;
	else 
#ifndef ARM_TP_ADDRESS
		set_tls(args);
#else
		*(register_t *)ARM_TP_ADDRESS = (register_t)args;
#endif
	return (0);
}

static int
arm32_get_tp(struct thread *td, void *args)
{

	if (td != curthread)
		td->td_retval[0] = td->td_md.md_tp;
	else
#ifndef ARM_TP_ADDRESS
		td->td_retval[0] = (register_t)get_tls();
#else
		td->td_retval[0] = *(register_t *)ARM_TP_ADDRESS;
#endif
	return (0);
}

int
sysarch(td, uap)
	struct thread *td;
	register struct sysarch_args *uap;
{
	int error;

#ifdef CAPABILITY_MODE
	/*
	 * When adding new operations, add a new case statement here to
	 * explicitly indicate whether or not the operation is safe to
	 * perform in capability mode.
	 */
	if (IN_CAPABILITY_MODE(td)) {
		switch (uap->op) {
		case ARM_SYNC_ICACHE:
		case ARM_DRAIN_WRITEBUF:
		case ARM_SET_TP:
		case ARM_GET_TP:
			break;

		default:
#ifdef KTRACE
			if (KTRPOINT(td, KTR_CAPFAIL))
				ktrcapfail(CAPFAIL_SYSCALL, 0, 0);
#endif
			return (ECAPMODE);
		}
	}
#endif

	switch (uap->op) {
	case ARM_SYNC_ICACHE:
		error = arm32_sync_icache(td, uap->parms);
		break;
	case ARM_DRAIN_WRITEBUF:
		error = arm32_drain_writebuf(td, uap->parms);
		break;
	case ARM_SET_TP:
		error = arm32_set_tp(td, uap->parms);
		break;
	case ARM_GET_TP:
		error = arm32_get_tp(td, uap->parms);
		break;
	default:
		error = EINVAL;
		break;
	}
	return (error);
}