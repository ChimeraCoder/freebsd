
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
#include <ucontext.h>
#include <machine/fsr.h>

#include "libpthread_db.h"

void
pt_reg_to_ucontext(const struct reg *r, ucontext_t *uc)
{

	memcpy(&uc->uc_mcontext, r, MIN(sizeof(uc->uc_mcontext), sizeof(*r)));
}

void
pt_ucontext_to_reg(const ucontext_t *uc, struct reg *r)
{

	memcpy(r, &uc->uc_mcontext, MIN(sizeof(uc->uc_mcontext), sizeof(*r)));
}

void
pt_fpreg_to_ucontext(const struct fpreg* r, ucontext_t *uc)
{
	mcontext_t *mc = &uc->uc_mcontext;

	memcpy(mc->mc_fp, r->fr_regs, MIN(sizeof(mc->mc_fp),
	    sizeof(r->fr_regs)));
	mc->mc_fsr = r->fr_fsr;
	mc->mc_gsr = r->fr_gsr;
	mc->mc_fprs |= FPRS_FEF;
}

void
pt_ucontext_to_fpreg(const ucontext_t *uc, struct fpreg *r)
{
	const mcontext_t *mc = &uc->uc_mcontext;

	if ((mc->mc_fprs & FPRS_FEF) != 0) {
		memcpy(r->fr_regs, mc->mc_fp, MIN(sizeof(mc->mc_fp),
		    sizeof(r->fr_regs)));
		r->fr_fsr = mc->mc_fsr;
		r->fr_gsr = mc->mc_gsr;
	} else
		memset(r, 0, sizeof(*r));
}

void
pt_md_init(void)
{

}

int
pt_reg_sstep(struct reg *reg __unused, int step __unused)
{

	return (0);
}