
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
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/systm.h>
#include <sys/sysent.h>
#include <sys/proc.h>

#include <machine/cpu.h>

#include <i386/ibcs2/ibcs2_types.h>
#include <i386/ibcs2/ibcs2_signal.h>
#include <i386/ibcs2/ibcs2_proto.h>
#include <i386/ibcs2/ibcs2_isc_syscall.h>

extern struct sysent isc_sysent[];

int
ibcs2_isc(struct thread *td, struct ibcs2_isc_args *uap)
{
	struct trapframe *tf = td->td_frame;
        struct sysent *callp;
        u_int code;
	int error;

	code = (tf->tf_eax & 0xffffff00) >> 8;
	callp = &isc_sysent[code];

	if (code < IBCS2_ISC_MAXSYSCALL)
		error = (*callp->sy_call)(td, (void *)uap);
	else
		error = ENOSYS;
	return (error);
}