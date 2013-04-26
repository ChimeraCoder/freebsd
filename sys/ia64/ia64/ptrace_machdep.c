
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
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/ptrace.h>
#include <machine/frame.h>

int
cpu_ptrace(struct thread *td, int req, void *addr, int data)
{
	struct trapframe *tf;
	uint64_t *kstack;
	int error;

	error = EINVAL;
	tf = td->td_frame;

	switch (req) {
	case PT_GETKSTACK:
		if (data >= 0 && data < (tf->tf_special.ndirty >> 3)) {
			kstack = (uint64_t*)(td->td_kstack +
			    (tf->tf_special.bspstore & 0x1ffUL));
			error = copyout(kstack + data, addr, 8);
		}
		break;
	case PT_SETKSTACK:
		if (data >= 0 && data < (tf->tf_special.ndirty >> 3)) {
			kstack = (uint64_t*)(td->td_kstack +
			    (tf->tf_special.bspstore & 0x1ffUL));
			error = copyin(addr, kstack + data, 8);
		}
		break;
	}

	return (error);
}