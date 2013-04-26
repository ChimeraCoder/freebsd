
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
#include <sys/sbuf.h>

#include <fs/pseudofs/pseudofs.h>
#include <fs/procfs/procfs.h>

int
procfs_doprocnote(PFS_FILL_ARGS)
{
	sbuf_trim(sb);
	sbuf_finish(sb);
	/* send to process's notify function */
	return (EOPNOTSUPP);
}