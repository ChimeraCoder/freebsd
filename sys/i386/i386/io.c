
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

#include <machine/frame.h>
#include <machine/iodev.h>
#include <machine/psl.h>

int
iodev_open(struct thread *td)
{

	td->td_frame->tf_eflags |= PSL_IOPL;
	return (0);
}

int
iodev_close(struct thread *td)
{

	td->td_frame->tf_eflags &= ~PSL_IOPL;
	return (0);
}

/* ARGSUSED */
int
iodev_ioctl(u_long cmd __unused, caddr_t data __unused)
{

	return (ENOIOCTL);
}