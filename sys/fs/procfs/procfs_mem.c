
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
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/ptrace.h>
#include <sys/systm.h>
#include <sys/uio.h>

#include <fs/pseudofs/pseudofs.h>
#include <fs/procfs/procfs.h>

/*
 * Copy data in and out of the target process.
 * We do this by mapping the process's page into
 * the kernel and then doing a uiomove direct
 * from the kernel address space.
 */
int
procfs_doprocmem(PFS_FILL_ARGS)
{
	int error;

	if (uio->uio_resid == 0)
		return (0);

	PROC_LOCK(p);
	error = p_candebug(td, p);
	PROC_UNLOCK(p);
	if (error == 0)
		error = proc_rwmem(p, uio);

	return (error);
}