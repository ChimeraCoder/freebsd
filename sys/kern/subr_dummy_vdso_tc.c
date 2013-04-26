
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
#include <sys/vdso.h>

uint32_t
cpu_fill_vdso_timehands(struct vdso_timehands *vdso_th)
{

	return (0);
}

#ifdef COMPAT_FREEBSD32
uint32_t
cpu_fill_vdso_timehands32(struct vdso_timehands32 *vdso_th32)
{

	return (0);
}
#endif