
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
#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/sysproto.h>

#include "opt_compat.h"

#ifdef COMPAT_FREEBSD32
#include <compat/freebsd32/freebsd32_proto.h>

int
freebsd32_sysarch(struct thread *td, struct freebsd32_sysarch_args *uap)
{

	return (EINVAL);
}
#endif

int
sysarch(struct thread *td, struct sysarch_args *uap)
{

	return (EINVAL);
}