
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

#ifndef lint
#if 0
static char sccsid[] = "@(#)util.c	8.3 (Berkeley) 4/2/94";
#endif
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "stty.h"
#include "extern.h"

/*
 * Gross, but since we're changing the control descriptor from 1 to 0, most
 * users will be probably be doing "stty > /dev/sometty" by accident.  If 1
 * and 2 are both ttys, but not the same, assume that 1 was incorrectly
 * redirected.
 */
void
checkredirect(void)
{
	struct stat sb1, sb2;

	if (isatty(STDOUT_FILENO) && isatty(STDERR_FILENO) &&
	    !fstat(STDOUT_FILENO, &sb1) && !fstat(STDERR_FILENO, &sb2) &&
	    (sb1.st_rdev != sb2.st_rdev))
warnx("stdout appears redirected, but stdin is the control descriptor");
}