
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
static char sccsid[] = "@(#)misc.c	8.3 (Berkeley) 4/2/94";
#endif
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/time.h>

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dd.h"
#include "extern.h"

void
summary(void)
{
	struct timeval tv;
	double secs;
	char buf[100];

	(void)gettimeofday(&tv, NULL);
	secs = tv.tv_sec + tv.tv_usec * 1e-6 - st.start;
	if (secs < 1e-6)
		secs = 1e-6;
	/* Use snprintf(3) so that we don't reenter stdio(3). */
	(void)snprintf(buf, sizeof(buf),
	    "%ju+%ju records in\n%ju+%ju records out\n",
	    st.in_full, st.in_part, st.out_full, st.out_part);
	(void)write(STDERR_FILENO, buf, strlen(buf));
	if (st.swab) {
		(void)snprintf(buf, sizeof(buf), "%ju odd length swab %s\n",
		     st.swab, (st.swab == 1) ? "block" : "blocks");
		(void)write(STDERR_FILENO, buf, strlen(buf));
	}
	if (st.trunc) {
		(void)snprintf(buf, sizeof(buf), "%ju truncated %s\n",
		     st.trunc, (st.trunc == 1) ? "block" : "blocks");
		(void)write(STDERR_FILENO, buf, strlen(buf));
	}
	(void)snprintf(buf, sizeof(buf),
	    "%ju bytes transferred in %.6f secs (%.0f bytes/sec)\n",
	    st.bytes, secs, st.bytes / secs);
	(void)write(STDERR_FILENO, buf, strlen(buf));
}

/* ARGSUSED */
void
summaryx(int notused __unused)
{
	int save_errno = errno;

	summary();
	errno = save_errno;
}

/* ARGSUSED */
void
terminate(int sig)
{

	summary();
	_exit(sig == 0 ? 0 : 1);
}