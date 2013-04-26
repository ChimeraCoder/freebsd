
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

/*
 * IBCS2 message compatibility module.
 *
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/syscallsubr.h>
#include <sys/sysproto.h>

#include <i386/ibcs2/ibcs2_types.h>
#include <i386/ibcs2/ibcs2_signal.h>
#include <i386/ibcs2/ibcs2_util.h>
#include <i386/ibcs2/ibcs2_proto.h>


int
ibcs2_getmsg(td, uap)
	struct thread *td;
	struct ibcs2_getmsg_args *uap;
{
	return 0; /* fake */
}

int
ibcs2_putmsg(td, uap)
	struct thread *td;
	struct ibcs2_putmsg_args *uap;
{
	return 0; /* fake */
}