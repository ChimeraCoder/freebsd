
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
#include <sys/pmc.h>

#include <machine/pmc_mdep.h>

struct pmc_mdep *
pmc_md_initialize()
{
	return NULL;
}

void
pmc_md_finalize(struct pmc_mdep *md)
{
	(void) md;
}

int
pmc_save_kernel_callchain(uintptr_t *cc, int maxsamples,
    struct trapframe *tf)
{
	(void) cc;
	(void) maxsamples;
	(void) tf;
	return (0);
}

int
pmc_save_user_callchain(uintptr_t *cc, int maxsamples,
    struct trapframe *tf)
{
	(void) cc;
	(void) maxsamples;
	(void) tf;
	return (0);
}