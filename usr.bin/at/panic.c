
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

/* System Headers */

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Local headers */

#include "panic.h"
#include "privs.h"
#include "at.h"

/* External variables */

/* Global functions */

void
panic(const char *a)
{
/* Something fatal has happened, print error message and exit.
 */
	if (fcreated) {
		PRIV_START
		unlink(atfile);
		PRIV_END
	}

	errx(EXIT_FAILURE, "%s", a);
}

void
perr(const char *a)
{
/* Some operating system error; print error message and exit.
 */
	int serrno = errno;

	if (fcreated) {
		PRIV_START
		unlink(atfile);
		PRIV_END
	}

	errno = serrno;
	err(EXIT_FAILURE, "%s", a);
}

void
usage(void)
{
	/* Print usage and exit. */
    fprintf(stderr, "usage: at [-q x] [-f file] [-m] time\n"
		    "       at -c job [job ...]\n"
		    "       at [-f file] -t [[CC]YY]MMDDhhmm[.SS]\n"
		    "       at -r job [job ...]\n"
		    "       at -l -q queuename\n"
		    "       at -l [job ...]\n"
		    "       atq [-q x] [-v]\n"
		    "       atrm job [job ...]\n"
		    "       batch [-f file] [-m]\n");
    exit(EXIT_FAILURE);
}