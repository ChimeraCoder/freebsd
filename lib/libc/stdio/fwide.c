
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

#include "namespace.h"
#include <errno.h>
#include <stdio.h>
#include <wchar.h>
#include "un-namespace.h"
#include "libc_private.h"
#include "local.h"

int
fwide(FILE *fp, int mode)
{
	int m;

	FLOCKFILE(fp);
	/* Only change the orientation if the stream is not oriented yet. */
	if (mode != 0 && fp->_orientation == 0)
		fp->_orientation = mode > 0 ? 1 : -1;
	m = fp->_orientation;
	FUNLOCKFILE(fp);

	return (m);
}