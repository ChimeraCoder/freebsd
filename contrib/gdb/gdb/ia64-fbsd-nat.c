
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

#include "defs.h"
#include "inferior.h"
#include "regcache.h"

#include <sys/types.h>
#include <sys/ptrace.h>
#include <machine/reg.h>

#ifdef HAVE_SYS_PROCFS_H
#include <sys/procfs.h>
#endif

#ifndef HAVE_GREGSET_T
typedef struct reg gregset_t;
#endif

#ifndef HAVE_FPREGSET_T
typedef struct fpreg fpregset_t;
#endif

#include "gregset.h"

#define	FPREG_SUPPLIES(r)  ((r) >= IA64_FR0_REGNUM && (r) <= IA64_FR127_REGNUM)
#define	GREG_SUPPLIES(r)   (!FPREG_SUPPLIES(r))

void
fetch_inferior_registers (int regno)
{
  union {
    fpregset_t fpr;
    gregset_t r;
  } regs;

  if (regno == -1 || GREG_SUPPLIES(regno))
    {
      if (ptrace (PT_GETREGS, PIDGET(inferior_ptid),
		  (PTRACE_ARG3_TYPE)&regs.r, 0) == -1)
	perror_with_name ("Couldn't get registers");
      supply_gregset (&regs.r);
    }

  if (regno == -1 || FPREG_SUPPLIES(regno))
    {
      if (ptrace (PT_GETFPREGS, PIDGET(inferior_ptid),
		  (PTRACE_ARG3_TYPE)&regs.fpr, 0) == -1)
	perror_with_name ("Couldn't get FP registers");
      supply_fpregset (&regs.fpr);
    }
}

void
store_inferior_registers (int regno)
{
  union {
    fpregset_t fpr;
    gregset_t r;
  } regs;

  if (regno == -1 || GREG_SUPPLIES(regno))
    {
      if (ptrace (PT_GETREGS, PIDGET(inferior_ptid),
		  (PTRACE_ARG3_TYPE)&regs.r, 0) == -1)
	perror_with_name ("Couldn't get registers");
      fill_gregset (&regs.r, regno);
      if (ptrace (PT_SETREGS, PIDGET(inferior_ptid),
		  (PTRACE_ARG3_TYPE)&regs.r, 0) == -1)
	perror_with_name ("Couldn't get registers");
      if (regno != -1)
	return;
    }

  if (regno == -1 || FPREG_SUPPLIES(regno))
    {
      if (ptrace (PT_GETFPREGS, PIDGET(inferior_ptid),
		  (PTRACE_ARG3_TYPE)&regs.fpr, 0) == -1)
	perror_with_name ("Couldn't get FP registers");
      fill_fpregset (&regs.fpr, regno);
      if (ptrace (PT_SETFPREGS, PIDGET(inferior_ptid),
		  (PTRACE_ARG3_TYPE)&regs.fpr, 0) == -1)
	perror_with_name ("Couldn't get FP registers");
      if (regno != -1)
	return;
    }
}

LONGEST ia64_fbsd_xfer_dirty (struct target_ops *ops, enum target_object obj,
			      const char *annex, void *rbuf, const void *wbuf,
			      ULONGEST ofs, LONGEST len)
{
  if (len != 8)
    return (-1);
  if (rbuf != NULL) {
    if (ptrace (PT_GETKSTACK, PIDGET(inferior_ptid), (PTRACE_ARG3_TYPE)rbuf,
		ofs >> 3) == -1) {
      perror_with_name ("Couldn't read dirty register");
      return (-1);
    }
  } else {
    if (ptrace (PT_SETKSTACK, PIDGET(inferior_ptid), (PTRACE_ARG3_TYPE)wbuf,
		ofs >> 3) == -1) {
      perror_with_name ("Couldn't write dirty register");
      return (-1);
    }
  }
  return (len);
}

void
_initialize_ia64_fbsd_nat (void)
{
}