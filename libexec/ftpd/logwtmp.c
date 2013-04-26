
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
static char sccsid[] = "@(#)logwtmp.c	8.1 (Berkeley) 6/4/93";
#endif
#endif /* not lint */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <libutil.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <utmpx.h>
#include "extern.h"

void
ftpd_logwtmp(char *id, char *user, struct sockaddr *addr)
{
	struct utmpx ut;

	memset(&ut, 0, sizeof(ut));

	if (user != NULL) {
		/* Log in. */
		ut.ut_type = USER_PROCESS;
		(void)strncpy(ut.ut_user, user, sizeof(ut.ut_user));
		if (addr != NULL)
			realhostname_sa(ut.ut_host, sizeof(ut.ut_host),
			    addr, addr->sa_len);
	} else {
		/* Log out. */
		ut.ut_type = DEAD_PROCESS;
	}

	ut.ut_pid = getpid();
	gettimeofday(&ut.ut_tv, NULL);
	(void)strncpy(ut.ut_id, id, sizeof(ut.ut_id));
	(void)strncpy(ut.ut_line, "ftpd", sizeof(ut.ut_line));

	pututxline(&ut);
}