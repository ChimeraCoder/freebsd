
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
#include <sys/procfs.h>
#include <ucontext.h>
#include <string.h>
#include <thread_db.h>
#include "libpthread_db.h"

void
pt_reg_to_ucontext(const struct reg *r, ucontext_t *uc)
{

	memcpy(uc->uc_mcontext.mc_regs, &r->r_regs[ZERO],
	    sizeof(uc->uc_mcontext.mc_regs));
	uc->uc_mcontext.mc_pc = r->r_regs[PC];
	uc->uc_mcontext.mullo = r->r_regs[MULLO];
	uc->uc_mcontext.mulhi = r->r_regs[MULHI];
}

void
pt_ucontext_to_reg(const ucontext_t *uc, struct reg *r)
{
	memcpy(&r->r_regs[ZERO], uc->uc_mcontext.mc_regs,
	    sizeof(uc->uc_mcontext.mc_regs));
	r->r_regs[PC] = uc->uc_mcontext.mc_pc;
	r->r_regs[MULLO] = uc->uc_mcontext.mullo;
	r->r_regs[MULHI] = uc->uc_mcontext.mulhi;
}

void
pt_fpreg_to_ucontext(const struct fpreg* r, ucontext_t *uc)
{

	memcpy(uc->uc_mcontext.mc_fpregs, r->r_regs,
	    sizeof(uc->uc_mcontext.mc_fpregs));
}

void
pt_ucontext_to_fpreg(const ucontext_t *uc, struct fpreg *r)
{

	memcpy(r->r_regs, uc->uc_mcontext.mc_fpregs,
	    sizeof(uc->uc_mcontext.mc_fpregs));
}

void
pt_md_init(void)
{
	/* Nothing to do */
}

int
pt_reg_sstep(struct reg *reg __unused, int step __unused)
{
	/*
	 * XXX: mips doesnt store single step info in any registers
	 */
	return (0);
}