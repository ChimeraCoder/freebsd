
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)rewind.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <errno.h>
#include <stdio.h>
#include "un-namespace.h"
#include "libc_private.h"
#include "local.h"

void
rewind(FILE *fp)
{
	int serrno = errno;

	/* make sure stdio is set up */
	if (!__sdidinit)
		__sinit();

	FLOCKFILE(fp);
	if (_fseeko(fp, (off_t)0, SEEK_SET, 1) == 0) {
		clearerr_unlocked(fp);
		errno = serrno;
	}
	FUNLOCKFILE(fp);
}