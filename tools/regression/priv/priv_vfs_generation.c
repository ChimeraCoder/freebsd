
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
 * Confirm that a generation number isn't returned by stat() when not running
 * with privilege.  In order to differentiate between a generation of 0 and
 * a generation not being returned, we have to create a temporary file known
 * to have a non-0 generation.  We try up to MAX_TRIES times, and then give
 * up, which is non-ideal, but better than not testing for a problem.
 */

#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "main.h"

static char fpath[1024];
static int fpath_initialized;

#define	MAX_TRIES	100

int
priv_vfs_generation_setup(int asroot, int injail, struct test *test)
{
	struct stat sb;
	int i;

	/*
	 * The kernel zeros the generation number field when an unprivileged
	 * user stats a file.  In order to distinguish the two cases, we
	 * therefore require a file that we know has a non-zero generation
	 * number.  We try up to MAX_TRIES times and otherwise fail.
	 */
	for (i = 0; i < MAX_TRIES; i++) {
		setup_file("priv_vfs_generation_setup: fpath", fpath,
		    UID_ROOT, GID_WHEEL, 0644);
		if (stat(fpath, &sb) < 0) {
			warn("priv_vfs_generation_setup: fstat(%s)", fpath);
			(void)unlink(fpath);
			return (-1);
		}
		if (sb.st_gen != 0) {
			fpath_initialized = 1;
			return (0);
		}
	}
	warnx("priv_vfs_generation_setup: unable to create gen file");
	return (-1);
}

void
priv_vfs_generation(int asroot, int injail, struct test *test)
{
	struct stat sb;
	int error;

	error = stat(fpath, &sb);
	if (error < 0)
		warn("priv_vfs_generation(asroot, injail) stat");

	if (sb.st_gen == 0) {
		error = -1;
		errno = EPERM;
	} else
		error = 0;
	if (asroot && injail)
		expect("priv_vfs_generation(asroot, injail)", error, -1,
		    EPERM);
	if (asroot && !injail)
		expect("priv_vfs_generation(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_vfs_generation(!asroot, injail)", error, -1,
		    EPERM);
	if (!asroot && !injail)
		expect("priv_vfs_generation(!asroot, !injail)", error, -1,
		    EPERM);
}

void
priv_vfs_generation_cleanup(int asroot, int injail, struct test *test)
{

	if (fpath_initialized) {
		(void)unlink(fpath);
		fpath_initialized = 0;
	}
}