
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/ktr.h>
#include <sys/proc.h>

#include <machine/frame.h>
#include <machine/pcb.h>

CTASSERT((1 << RW_SHIFT) == sizeof(struct rwindow));

int
rwindow_load(struct thread *td, struct trapframe *tf, int n)
{
	struct rwindow rw;
	u_long usp;
	int error;
	int i;

	CTR3(KTR_TRAP, "rwindow_load: td=%p (%s) n=%d",
	    td, td->td_proc->p_comm, n);

	/*
	 * In case current window is still only on-chip, push it out;
	 * if it cannot get all the way out, we cannot continue either.
	 */
	if ((error = rwindow_save(td)) != 0)
		return (error);
	usp = tf->tf_out[6];
	for (i = 0; i < n; i++) {
		CTR1(KTR_TRAP, "rwindow_load: usp=%#lx", usp);
		usp += SPOFF;
		if ((error = (usp & 0x7)) != 0)
			break;
		error = copyin((void *)usp, &rw, sizeof rw);
		usp = rw.rw_in[6];
	}
	CTR1(KTR_TRAP, "rwindow_load: error=%d", error);
	return (error == 0 ? 0 : SIGILL);
}

int
rwindow_save(struct thread *td)
{
	struct rwindow *rw;
	struct pcb *pcb;
	u_long *ausp;
	u_long usp;
	int error;
	int i;

	pcb = td->td_pcb;
	CTR3(KTR_TRAP, "rwindow_save: td=%p (%s) nsaved=%d", td,
	    td->td_proc->p_comm, pcb->pcb_nsaved);

	flushw();
	KASSERT(pcb->pcb_nsaved < MAXWIN,
	    ("rwindow_save: pcb_nsaved > MAXWIN"));
	if ((i = pcb->pcb_nsaved) == 0)
		return (0);
	ausp = pcb->pcb_rwsp;
	rw = pcb->pcb_rw;
	error = 0;
	do {
		usp = *ausp;
		CTR1(KTR_TRAP, "rwindow_save: usp=%#lx", usp);
		usp += SPOFF;
		if ((error = (usp & 0x7)) != 0)
			break;
		error = copyout(rw, (void *)usp, sizeof *rw);
		if (error)
			break;
		ausp++;
		rw++;
	} while (--i > 0);
	CTR1(KTR_TRAP, "rwindow_save: error=%d", error);
	if (error == 0)
		pcb->pcb_nsaved = 0;
	return (error == 0 ? 0 : SIGILL);
}