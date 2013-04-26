
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

#include <sys/fcntl.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/syscallsubr.h>

#include <i386/ibcs2/ibcs2_util.h>


const char      ibcs2_emul_path[] = "/compat/ibcs2";

/*
 * Search an alternate path before passing pathname arguments on
 * to system calls. Useful for keeping a separate 'emulation tree'.
 *
 * If cflag is set, we check if an attempt can be made to create
 * the named file, i.e. we check if the directory it should
 * be in exists.
 */
int
ibcs2_emul_find(struct thread *td, char *path, enum uio_seg pathseg,
    char **pbuf, int cflag)
{

	return (kern_alternate_path(td, ibcs2_emul_path, path, pathseg, pbuf,
	    cflag, AT_FDCWD));
}