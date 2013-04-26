
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
 * Test privilege check on /dev/io.  By default, the permissions also protect
 * against non-superuser access, so this program will modify permissions on
 * /dev/io to allow world access, and revert the change on exit.  This is not
 * good for run-time security, but is necessary to test the checks properly.
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "main.h"

#define	NEW_PERMS	0666
#define	DEV_IO		"/dev/io"
#define	EXPECTED_PERMS	0600

static int initialized;
static mode_t saved_perms;

int
priv_io_setup(int asroot, int asjail, struct test *test)
{
	struct stat sb;

	if (stat(DEV_IO, &sb) < 0) {
		warn("priv_io_setup: stat(%s)", DEV_IO);
		return (-1);
	}
	saved_perms = sb.st_mode & ALLPERMS;
	if (saved_perms != EXPECTED_PERMS) {
		warnx("priv_io_setup: perms = 0%o; expected 0%o",
		    saved_perms, EXPECTED_PERMS);
		return (-1);
	}
	if (chmod(DEV_IO, NEW_PERMS) < 0) {
		warn("priv_io_setup: chmod(%s, 0%o)", DEV_IO, NEW_PERMS);
		return (-1);
	}
	initialized = 1;
	return (0);
}

void
priv_io(int asroot, int injail, struct test *test)
{
	int error, fd;

	fd = open(DEV_IO, O_RDONLY);
	if (fd < 0)
		error = -1;
	else
		error = 0;
	if (asroot && injail)
		expect("priv_io(asroot, injail)", error, -1, EPERM);
	if (asroot && !injail)
		expect("priv_io(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_io(!asroot, injail)", error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_io(!asroot, !injail)", error, -1, EPERM);
	if (fd != -1)
		close(fd);
}

void
priv_io_cleanup(int asroot, int asjail, struct test *test)
{

	if (!initialized)
		return;
	if (chmod(DEV_IO, saved_perms) < 0)
		err(-1, "priv_io_cleanup: chmod(%s, 0%o)", DEV_IO,
		    saved_perms);
	initialized = 0;
}