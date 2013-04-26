
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
#include <sys/signal.h>
#include <sys/ucontext.h>

#include <errno.h>
#include <stddef.h>

__weak_reference(__swapcontext, swapcontext);

int
__swapcontext(ucontext_t *oucp, const ucontext_t *ucp)
{
	int ret;

	if ((oucp == NULL) || (ucp == NULL)) {
		errno = EINVAL;
		return (-1);
	}
	oucp->uc_flags &= ~UCF_SWAPPED;
	ret = getcontext(oucp);
	if ((ret == 0) && !(oucp->uc_flags & UCF_SWAPPED)) {
		oucp->uc_flags |= UCF_SWAPPED;
		ret = setcontext(ucp);
	}
	return (ret);
}