
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

/*
 * Test program for perror() as specified by IEEE Std. 1003.1-2001 and
 * ISO/IEC 9899:1999.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <limits.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void cleanup(void);
static char tmpfil[PATH_MAX];

int
main(int argc, char *argv[])
{
	char lbuf[512];
	int i;
	char *s;

	printf("1..1\n");

	strcpy(tmpfil, _PATH_TMP "perror.XXXXXXXX");
	if (mkstemp(tmpfil) < 0)
		err(1, "mkstemp");
	atexit(cleanup);
	/* Reopen stderr on a file descriptor other than 2. */
	fclose(stderr);
	for (i = 0; i < 3; i++)
		dup(0);
	if (freopen(tmpfil, "r+", stderr) == NULL)
		err(1, "%s", tmpfil);

	/*
	 * Test that perror() doesn't call strerror() (4.4BSD bug),
	 * the two ways of omitting a program name, and the formatting when
	 * a program name is specified.
	 */
	s = strerror(ENOENT);
	assert(strcmp(s, "No such file or directory") == 0);
	errno = EPERM;
	perror(NULL);
	perror("");
	perror("test-perror");
	assert(strcmp(s, "No such file or directory") == 0);

	/*
	 * Read it back to check...
	 */
	rewind(stderr);
	s = fgets(lbuf, sizeof(lbuf), stderr);
	assert(s != NULL);
	assert(strcmp(s, "Operation not permitted\n") == 0);
	s = fgets(lbuf, sizeof(lbuf), stderr);
	assert(s != NULL);
	assert(strcmp(s, "Operation not permitted\n") == 0);
	s = fgets(lbuf, sizeof(lbuf), stderr);
	assert(s != NULL);
	assert(strcmp(s, "test-perror: Operation not permitted\n") == 0);
	s = fgets(lbuf, sizeof(lbuf), stderr);
	assert(s == NULL);
	fclose(stderr);

	printf("ok 1 - perror()\n");

	return (0);
}

static void
cleanup(void)
{

	unlink(tmpfil);
}