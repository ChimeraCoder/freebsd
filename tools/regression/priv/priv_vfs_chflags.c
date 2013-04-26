
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
 * Test privileges associated with setting file flags on files; whether or
 * not it requires privilege depends on the flag, and some flags cannot be
 * set in jail at all.
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "main.h"

static char fpath[1024];
static int fpath_initialized;

/*
 * For chflags, we consider three dimmensions: process owner, file owner, and
 * flag type.  The calling framework handles variations in process owner; the
 * rest are handled via multiple tests.  One cleanup function is used.
 */
static u_long
getflags(char *fpathp)
{
	struct stat sb;

	if (stat(fpathp, &sb) < 0)
		err(-1, "stat(%s)", fpathp);

	return (sb.st_flags);
}

int
priv_vfs_chflags_froot_setup(int asroot, int injail, struct test *test)
{

	setup_file("priv_vfs_chflags_froot_setup: fpath", fpath, UID_ROOT,
	    GID_WHEEL, 0600);
	fpath_initialized = 1;
	return (0);
}

int
priv_vfs_chflags_fowner_setup(int asroot, int injail,
    struct test *test)
{

	setup_file("priv_vfs_chflags_fowner_setup: fpath", fpath, UID_OWNER,
	    GID_OWNER, 0600);
	fpath_initialized = 1;
	return (0);
}

int
priv_vfs_chflags_fother_setup(int asroot, int injail,
    struct test *test)
{

	setup_file("priv_vfs_chflags_fowner_setup: fpath", fpath, UID_OTHER,
	    GID_OTHER, 0600);
	fpath_initialized = 1;
	return (0);
}

void
priv_vfs_chflags_froot_uflags(int asroot, int injail,
    struct test *test)
{
	u_long flags;
	int error;

	flags = getflags(fpath);
	flags |= UF_NODUMP;
	error = chflags(fpath, flags);
	if (asroot && injail)
		expect("priv_vfs_chflags_froot_uflags(asroot, injail)",
		    error, 0, 0);
	if (asroot && !injail)
		expect("priv_vfs_chflags_froot_uflags(asroot, !injail)",
		    error, 0, 0);
	if (!asroot && injail)
		expect("priv_vfs_chflags_froot_uflags(!asroot, injail)",
		    error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_vfs_chflags_froot_uflags(!asroot, !injail)",
		    error, -1, EPERM);
}

void
priv_vfs_chflags_fowner_uflags(int asroot, int injail,
    struct test *test)
{
	u_long flags;
	int error;

	flags = getflags(fpath);
	flags |= UF_NODUMP;
	error = chflags(fpath, flags);
	if (asroot && injail)
		expect("priv_vfs_chflags_fowner_uflags(asroot, injail)",
		    error, 0, 0);
	if (asroot && !injail)
		expect("priv_vfs_chflags_fowner_uflags(asroot, !injail)",
		    error, 0, 0);
	if (!asroot && injail)
		expect("priv_vfs_chflags_fowner_uflags(!asroot, injail)",
		    error, 0, 0);
	if (!asroot && !injail)
		expect("priv_vfs_chflags_fowner_uflags(!asroot, !injail)",
		    error, 0, 0);
}

void
priv_vfs_chflags_fother_uflags(int asroot, int injail,
    struct test *test)
{
	u_long flags;
	int error;

	flags = getflags(fpath);
	flags |= UF_NODUMP;
	error = chflags(fpath, flags);
	if (asroot && injail)
		expect("priv_vfs_chflags_fother_uflags(asroot, injail)",
		    error, 0, 0);
	if (asroot && !injail)
		expect("priv_vfs_chflags_fother_uflags(asroot, !injail)",
		    error, 0, 0);
	if (!asroot && injail)
		expect("priv_vfs_chflags_fother_uflags(!asroot, injail)",
		    error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_vfs_chflags_fother_uflags(!asroot, !injail)",
		    error, -1, EPERM);
}

void
priv_vfs_chflags_froot_sflags(int asroot, int injail,
    struct test *test)
{
	u_long flags;
	int error;

	flags = getflags(fpath);
	flags |= SF_ARCHIVED;
	error = chflags(fpath, flags);
	if (asroot && injail)
		expect("priv_vfs_chflags_froot_sflags(asroot, injail)",
		    error, -1, EPERM);
	if (asroot && !injail)
		expect("priv_vfs_chflags_froot_sflags(asroot, !injail)",
		    error, 0, 0);
	if (!asroot && injail)
		expect("priv_vfs_chflags_froot_sflags(!asroot, injail)",
		    error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_vfs_chflags_froot_sflags(!asroot, !injail)",
		    error, -1, EPERM);
}

void
priv_vfs_chflags_fowner_sflags(int asroot, int injail,
    struct test *test)
{
	u_long flags;
	int error;

	flags = getflags(fpath);
	flags |= SF_ARCHIVED;
	error = chflags(fpath, flags);
	if (asroot && injail)
		expect("priv_vfs_chflags_fowner_sflags(asroot, injail)",
		    error, -1, EPERM);
	if (asroot && !injail)
		expect("priv_vfs_chflags_fowner_sflags(asroot, !injail)",
		    error, 0, 0);
	if (!asroot && injail)
		expect("priv_vfs_chflags_fowner_sflags(!asroot, injail)",
		    error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_vfs_chflags_fowner_sflags(!asroot, !injail)",
		    error, -1, EPERM);
}

void
priv_vfs_chflags_fother_sflags(int asroot, int injail,
    struct test *test)
{
	u_long flags;
	int error;

	flags = getflags(fpath);
	flags |= SF_ARCHIVED;
	error = chflags(fpath, flags);
	if (asroot && injail)
		expect("priv_vfs_chflags_fother_sflags(asroot, injail)",
		    error, -1, EPERM);
	if (asroot && !injail)
		expect("priv_vfs_chflags_fother_sflags(asroot, !injail)",
		    error, 0, 0);
	if (!asroot && injail)
		expect("priv_vfs_chflags_fother_sflags(!asroot, injail)",
		    error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_vfs_chflags_fother_sflags(!asroot, !injail)",
		    error, -1, EPERM);
}

void
priv_vfs_chflags_cleanup(int asroot, int injail, struct test *test)
{

	if (fpath_initialized) {
		(void)chflags(fpath, 0);
		(void)unlink(fpath);
		fpath_initialized = 0;
	}
}