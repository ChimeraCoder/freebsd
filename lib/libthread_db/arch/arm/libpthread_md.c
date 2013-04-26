
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
#include <string.h>
#include <thread_db.h>

#include "libpthread_db.h"

void
pt_reg_to_ucontext(const struct reg *r, ucontext_t *uc)
{
	mcontext_t *mc = &uc->uc_mcontext;
	__greg_t *gr = mc->__gregs;

	gr[_REG_R0] = r->r[0];
	gr[_REG_R1] = r->r[1];
	gr[_REG_R2] = r->r[2];
	gr[_REG_R3] = r->r[3];
	gr[_REG_R4] = r->r[4];
	gr[_REG_R5] = r->r[5];
	gr[_REG_R6] = r->r[6];
	gr[_REG_R7] = r->r[7];
	gr[_REG_R8] = r->r[8];
	gr[_REG_R9] = r->r[9];
	gr[_REG_R10] = r->r[10];
	gr[_REG_R11] = r->r[11];
	gr[_REG_R12] = r->r[12];
	gr[_REG_SP] = r->r_sp;
	gr[_REG_LR] = r->r_lr;
	gr[_REG_PC] = r->r_pc;
	gr[_REG_CPSR] = r->r_cpsr;
}

void
pt_ucontext_to_reg(const ucontext_t *uc, struct reg *r)
{
	const mcontext_t *mc = &uc->uc_mcontext;

	const __greg_t *gr = mc->__gregs;

	r->r[0] = gr[_REG_R0];
	r->r[1] = gr[_REG_R1];
	r->r[2] = gr[_REG_R2];
	r->r[3] = gr[_REG_R3];
	r->r[4] = gr[_REG_R4];
	r->r[5] = gr[_REG_R5];
	r->r[6] = gr[_REG_R6];
	r->r[7] = gr[_REG_R7];
	r->r[8] = gr[_REG_R8];
	r->r[9] = gr[_REG_R9];
	r->r[10] = gr[_REG_R10];
	r->r[11] = gr[_REG_R11];
	r->r[12] = gr[_REG_R12];
	r->r_sp = gr[_REG_SP];
	r->r_lr = gr[_REG_LR];
	r->r_pc = gr[_REG_PC];
	r->r_cpsr = gr[_REG_CPSR];
}

void
pt_fpreg_to_ucontext(const struct fpreg *r __unused, ucontext_t *uc)
{
	mcontext_t *mc = &uc->uc_mcontext;

	/* XXX */
	memset(&mc->__fpu.__fpregs, 0, sizeof(__fpregset_t));
}

void
pt_ucontext_to_fpreg(const ucontext_t *uc __unused, struct fpreg *r)
{

	/* XXX */
	memset(r, 0, sizeof(*r));
}

void
pt_md_init(void)
{
}

int
pt_reg_sstep(struct reg *reg __unused, int step __unused)
{

	/* XXX */
	return (0);
}