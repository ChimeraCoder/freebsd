
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
#include <machine/npx.h>
#include <string.h>
#include <thread_db.h>

#include "libpthread_db.h"

static int has_xmm_regs;

void
pt_reg_to_ucontext(const struct reg *r, ucontext_t *uc)
{
	memcpy(&uc->uc_mcontext.mc_fs, &r->r_fs, 18*4);
	uc->uc_mcontext.mc_gs = r->r_gs;
}

void
pt_ucontext_to_reg(const ucontext_t *uc, struct reg *r)
{
	memcpy(&r->r_fs, &uc->uc_mcontext.mc_fs, 18*4);
	r->r_gs = uc->uc_mcontext.mc_gs;
}

void
pt_fpreg_to_ucontext(const struct fpreg* r, ucontext_t *uc)
{
	if (!has_xmm_regs)
		memcpy(&uc->uc_mcontext.mc_fpstate, r,
			sizeof(struct save87));
	else {
		int i;
		struct savexmm *sx = (struct savexmm *)&uc->uc_mcontext.mc_fpstate;
		memcpy(&sx->sv_env, &r->fpr_env, sizeof(r->fpr_env));
		for (i = 0; i < 8; ++i)
			memcpy(&sx->sv_fp[i].fp_acc, &r->fpr_acc[i], 10);
	}
}

void
pt_ucontext_to_fpreg(const ucontext_t *uc, struct fpreg *r)
{
	if (!has_xmm_regs)
		memcpy(r, &uc->uc_mcontext.mc_fpstate, sizeof(struct save87));
	else {
		int i;
		struct savexmm *sx = (struct savexmm *)&uc->uc_mcontext.mc_fpstate;
		memcpy(&r->fpr_env, &sx->sv_env, sizeof(r->fpr_env));
		for (i = 0; i < 8; ++i)
			memcpy(&r->fpr_acc[i], &sx->sv_fp[i].fp_acc, 10);
	}
}

void
pt_fxsave_to_ucontext(const char* r, ucontext_t *uc)
{
	if (has_xmm_regs)
		memcpy(&uc->uc_mcontext.mc_fpstate, r, sizeof(struct savexmm));
}

void
pt_ucontext_to_fxsave(const ucontext_t *uc, char *r)
{
	if (has_xmm_regs)
		memcpy(r, &uc->uc_mcontext.mc_fpstate, sizeof(struct savexmm));
}

void
pt_md_init(void)
{
	ucontext_t uc;

	getcontext(&uc);
	if (uc.uc_mcontext.mc_fpformat == _MC_FPFMT_XMM)
	    has_xmm_regs = 1;
}

int
pt_reg_sstep(struct reg *reg, int step)
{
	unsigned int old;
	
	old = reg->r_eflags;
	if (step)
		reg->r_eflags |= 0x0100;
	else
		reg->r_eflags &= ~0x0100;
	return (old != reg->r_eflags); /* changed ? */
}