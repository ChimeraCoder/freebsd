
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
#include <sys/sysctl.h>

#include <i386/ibcs2/ibcs2_types.h>
#include <i386/ibcs2/ibcs2_signal.h>
#include <i386/ibcs2/ibcs2_util.h>
#include <i386/ibcs2/ibcs2_proto.h>

#define IBCS2_FP_NO     0       /* no fp support */
#define IBCS2_FP_SW     1       /* software emulator */
#define IBCS2_FP_287    2       /* 80287 FPU */
#define IBCS2_FP_387    3       /* 80387 FPU */

#define SI86_FPHW	40
#define STIME		54
#define SETNAME		56
#define SI86_MEM	65

extern int hw_float;

int
ibcs2_sysi86(struct thread *td, struct ibcs2_sysi86_args *args)
{
	switch (args->cmd) {
	case SI86_FPHW: {	/* Floating Point information */
		int val, error;

		if (hw_float)
			val = IBCS2_FP_387;
		else
			val = IBCS2_FP_NO;
		if ((error = copyout(&val, args->arg, sizeof(val))) != 0)
			return error;
		return 0;
		}

        case STIME:       /* set the system time given pointer to long */
	  /* gettimeofday; time.tv_sec = *args->arg; settimeofday */
	        return EINVAL;

	case SETNAME:  {  /* set hostname given string w/ len <= 7 chars */
	        int name[2];

		name[0] = CTL_KERN;
		name[1] = KERN_HOSTNAME;
		return (userland_sysctl(td, name, 2, 0, 0, 0, 
		    args->arg, 7, 0, 0));
	}

	case SI86_MEM:	/* size of physical memory */
		td->td_retval[0] = ctob(physmem);
		return 0;

	default:
#ifdef DIAGNOSTIC
		printf("IBCS2: 'sysi86' function %d(0x%x) "
			"not implemented yet\n", args->cmd, args->cmd);
#endif
		return EINVAL;
	}
}