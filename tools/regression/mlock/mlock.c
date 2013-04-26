
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

#include <sys/types.h>
#include <sys/mman.h>

#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define	NOBODY	"nobody"

/*
 * Simple exercise for the mlock() system call -- confirm that mlock() and
 * munlock() return success on an anonymously mapped memory page when running
 * with privilege; confirm that they fail with EPERM when running
 * unprivileged.
 */
int
main(int argc, char *argv[])
{
	struct passwd *pwd;
	int pagesize;
	char *page;

	if (geteuid() != 0)
		errx(-1, "mlock must run as root");

	errno = 0;
	pwd = getpwnam(NOBODY);
	if (pwd == NULL && errno == 0)
		errx(-1, "getpwnam: user \"%s\" not found", NOBODY);
	if (pwd == NULL)
		errx(-1, "getpwnam: %s", strerror(errno));
	if (pwd->pw_uid == 0)
		errx(-1, "getpwnam: user \"%s\" has uid 0", NOBODY);

	pagesize = getpagesize();
	page = mmap(NULL, pagesize, PROT_READ|PROT_WRITE, MAP_ANON, -1, 0);
	if (page == MAP_FAILED)
		errx(-1, "mmap: %s", strerror(errno));

	if (mlock(page, pagesize) < 0)
		errx(-1, "mlock privileged: %s", strerror(errno));

	if (munlock(page, pagesize) < 0)
		errx(-1, "munlock privileged: %s", strerror(errno));

	if (seteuid(pwd->pw_uid) < 0)
		errx(-1, "seteuid: %s", strerror(errno));

	if (mlock(page, pagesize) == 0)
		errx(-1, "mlock unprivileged: succeeded but shouldn't have");
	if (errno != EPERM)
		errx(-1, "mlock unprivileged: %s", strerror(errno));

	if (munlock(page, pagesize) == 0)
		errx(-1, "munlock unprivileged: succeeded but shouldn't have");
	if (errno != EPERM)
		errx(-1, "munlock unprivileged: %s", strerror(errno));

	return (0);
}