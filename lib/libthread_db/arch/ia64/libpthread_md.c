
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
#include <thread_db.h>
#include <ucontext.h>

#include "libpthread_db.h"

void
pt_reg_to_ucontext(const struct reg *r __unused, ucontext_t *uc __unused)
{
}

void
pt_ucontext_to_reg(const ucontext_t *uc __unused, struct reg *r __unused)
{
}

void
pt_fpreg_to_ucontext(const struct fpreg* r __unused, ucontext_t *uc __unused)
{
}

void
pt_ucontext_to_fpreg(const ucontext_t *uc __unused, struct fpreg *r __unused)
{
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