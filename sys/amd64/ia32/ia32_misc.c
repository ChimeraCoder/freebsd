
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

#include "opt_compat.h"

#include <sys/param.h>
#include <sys/mount.h>
#include <sys/proc.h>
#include <sys/socket.h>
#include <sys/sysent.h>
#include <sys/sysproto.h>
#include <sys/systm.h>
#include <sys/uio.h>

#include <machine/cpu.h>
#include <machine/sysarch.h>

#include <compat/freebsd32/freebsd32_util.h>
#include <compat/freebsd32/freebsd32.h>
#include <compat/freebsd32/freebsd32_proto.h>

int
freebsd32_sysarch(struct thread *td, struct freebsd32_sysarch_args *uap)
{
	struct sysarch_args uap1;
	struct i386_ldt_args uapl;
	struct i386_ldt_args32 uapl32;
	int error;

	if (uap->op == I386_SET_LDT || uap->op == I386_GET_LDT) {
		if ((error = copyin(uap->parms, &uapl32, sizeof(uapl32))) != 0)
			return (error);
		uap1.op = uap->op;
		uap1.parms = (char *)&uapl;
		uapl.start = uapl32.start;
		uapl.descs = (struct user_segment_descriptor *)(uintptr_t)
		    uapl32.descs;
		uapl.num = uapl32.num;
		return (sysarch_ldt(td, &uap1, UIO_SYSSPACE));
	} else {
		uap1.op = uap->op;
		uap1.parms = uap->parms;
		return (sysarch(td, &uap1));
	}
}

#ifdef COMPAT_43
int
ofreebsd32_getpagesize(struct thread *td,
    struct ofreebsd32_getpagesize_args *uap)
{

	td->td_retval[0] = IA32_PAGE_SIZE;
	return (0);
}
#endif