
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

#include "opt_compat.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/procfs.h>

int
fill_regs32(struct thread *td, struct reg32 *regs)
{

	bzero(regs, sizeof(*regs));
	return (EOPNOTSUPP);
}

int
set_regs32(struct thread *td, struct reg32 *regs)
{

	return (EOPNOTSUPP);
}

int
fill_fpregs32(struct thread *td, struct fpreg32 *regs)
{

	bzero(regs, sizeof(*regs));
	return (EOPNOTSUPP);
}

int
set_fpregs32(struct thread *td, struct fpreg32 *regs)
{

	return (EOPNOTSUPP);
}

int
fill_dbregs32(struct thread *td, struct dbreg32 *regs)
{

	bzero(regs, sizeof(*regs));
	return (EOPNOTSUPP);
}

int
set_dbregs32(struct thread *td, struct dbreg32 *regs)
{

	return (EOPNOTSUPP);
}