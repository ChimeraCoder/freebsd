
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
#include <sys/time.h>
#include <paths.h>
#include <sha.h>
#include <string.h>
#include <unistd.h>
#include <utmpx.h>
#include "ulog.h"

static void
ulog_fill(struct utmpx *utx, const char *line)
{
	SHA_CTX c;
	char id[SHA_DIGEST_LENGTH];

	/* Remove /dev/ component. */
	if (strncmp(line, _PATH_DEV, sizeof _PATH_DEV - 1) == 0)
		line += sizeof _PATH_DEV - 1;

	memset(utx, 0, sizeof *utx);

	utx->ut_pid = getpid();
	gettimeofday(&utx->ut_tv, NULL);
	strncpy(utx->ut_line, line, sizeof utx->ut_line);

	SHA1_Init(&c);
	SHA1_Update(&c, "libulog", 7);
	SHA1_Update(&c, utx->ut_line, sizeof utx->ut_line);
	SHA1_Final(id, &c);

	memcpy(utx->ut_id, id, MIN(sizeof utx->ut_id, sizeof id));
}

void
ulog_login(const char *line, const char *user, const char *host)
{
	struct utmpx utx;

	ulog_fill(&utx, line);
	utx.ut_type = USER_PROCESS;
	strncpy(utx.ut_user, user, sizeof utx.ut_user);
	if (host != NULL)
		strncpy(utx.ut_host, host, sizeof utx.ut_host);
	pututxline(&utx);
}

void
ulog_logout(const char *line)
{
	struct utmpx utx;

	ulog_fill(&utx, line);
	utx.ut_type = DEAD_PROCESS;
	pututxline(&utx);
}