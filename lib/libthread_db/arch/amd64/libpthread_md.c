
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

#include <sys/procfs.h>
#include <string.h>
#include <thread_db.h>
#include <ucontext.h>

#include "libpthread_db.h"

void
pt_reg_to_ucontext(const struct reg *r, ucontext_t *uc)
{
	mcontext_t *mc = &uc->uc_mcontext;

	mc->mc_rdi = r->r_rdi;
	mc->mc_rsi = r->r_rsi;
	mc->mc_rdx = r->r_rdx;
	mc->mc_rcx = r->r_rcx;
	mc->mc_r8 = r->r_r8;
	mc->mc_r9 = r->r_r9;
	mc->mc_rax = r->r_rax;
	mc->mc_rbx = r->r_rbx;
	mc->mc_rbp = r->r_rbp;
	mc->mc_r10 = r->r_r10;
	mc->mc_r11 = r->r_r11;
	mc->mc_r12 = r->r_r12;
	mc->mc_r13 = r->r_r13;
	mc->mc_r14 = r->r_r14;
	mc->mc_r15 = r->r_r15;
	mc->mc_rip = r->r_rip;
	mc->mc_cs = r->r_cs;
	mc->mc_rflags = r->r_rflags;
	mc->mc_rsp = r->r_rsp;
	mc->mc_ss = r->r_ss;
}

void
pt_ucontext_to_reg(const ucontext_t *uc, struct reg *r)
{
	const mcontext_t *mc = &uc->uc_mcontext;

	r->r_rdi = mc->mc_rdi;
	r->r_rsi = mc->mc_rsi;
	r->r_rdx = mc->mc_rdx;
	r->r_rcx = mc->mc_rcx;
	r->r_r8 = mc->mc_r8;
	r->r_r9 = mc->mc_r9;
	r->r_rax = mc->mc_rax;
	r->r_rbx = mc->mc_rbx;
	r->r_rbp = mc->mc_rbp;
	r->r_r10 = mc->mc_r10;
	r->r_r11 = mc->mc_r11;
	r->r_r12 = mc->mc_r12;
	r->r_r13 = mc->mc_r13;
	r->r_r14 = mc->mc_r14;
	r->r_r15 = mc->mc_r15;
	r->r_rip = mc->mc_rip;
	r->r_cs = mc->mc_cs;
	r->r_rflags = mc->mc_rflags;
	r->r_rsp = mc->mc_rsp;
	r->r_ss = mc->mc_ss;
}

void
pt_fpreg_to_ucontext(const struct fpreg* r, ucontext_t *uc)
{

	memcpy(&uc->uc_mcontext.mc_fpstate, r, sizeof(*r));
}

void
pt_ucontext_to_fpreg(const ucontext_t *uc, struct fpreg *r)
{

	memcpy(r, &uc->uc_mcontext.mc_fpstate, sizeof(*r));
}

void
pt_md_init(void)
{

	/* Nothing to do */
}

int
pt_reg_sstep(struct reg *reg, int step)
{
	register_t old;

	old = reg->r_rflags;
	if (step)
		reg->r_rflags |= 0x0100;
	else
		reg->r_rflags &= ~0x0100;
	return (old != reg->r_rflags); /* changed ? */
}