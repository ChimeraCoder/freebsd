
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
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/sbuf.h>
#include <sys/sysent.h>

#include <fs/pseudofs/pseudofs.h>
#include <fs/procfs/procfs.h>

int
procfs_doproctype(PFS_FILL_ARGS)
{
	static const char *none = "Not Available";

	if (p != NULL && p->p_sysent && p->p_sysent->sv_name)
		sbuf_printf(sb, "%s", p->p_sysent->sv_name);
	else
		sbuf_printf(sb, "%s", none);
	sbuf_putc(sb, '\n');
	return (0);
}