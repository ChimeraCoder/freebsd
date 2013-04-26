
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
#include <sys/proc.h>
#include <sys/sbuf.h>
#include <sys/uio.h>

#include <fs/pseudofs/pseudofs.h>
#include <fs/procfs/procfs.h>

int
procfs_doosrel(PFS_FILL_ARGS)
{
	const char *pp;
	int ov, osrel, i;

	if (uio == NULL)
		return (EOPNOTSUPP);
	if (uio->uio_rw == UIO_READ) {
		sbuf_printf(sb, "%d\n", p->p_osrel);
	} else {
		sbuf_trim(sb);
		sbuf_finish(sb);
		pp = sbuf_data(sb);
		osrel = 0;
		i = sbuf_len(sb);
		while (i--) {
			if (*pp < '0' || *pp > '9')
				return (EINVAL);
			ov = osrel * 10 + *pp++ - '0';
			if (ov < osrel)
				return (EINVAL);
			osrel = ov;
		}
		p->p_osrel = osrel;
	}
	return (0);
}