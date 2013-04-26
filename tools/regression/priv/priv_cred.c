
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
 * Confirm that various UID/GID/etc-related system calls require root
 * privilege in the absence of any saved/real/etc variations in the
 * credential.  It would be nice to also check cases where those bits of the
 * credential are more interesting.
 *
 * XXXRW: Add support for testing more diverse real/saved scenarios.
 */

#include <sys/types.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "main.h"

int
priv_cred_setup(int asroot, int injail, struct test *test)
{

	return (0);
}

void
priv_cred_setuid(int asroot, int injail, struct test *test)
{
	int error;

	error = setuid(UID_OTHER);
	if (asroot && injail)
		expect("priv_setuid(asroot, injail)", error, 0, 0);
	if (asroot && !injail)
		expect("priv_setuid(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_setuid(!asroot, injail)", error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_setuid(!asroot, !injail)", error, -1, EPERM);
}

void
priv_cred_seteuid(int asroot, int injail, struct test *test)
{
	int error;

	error = seteuid(UID_OTHER);
	if (asroot && injail)
		expect("priv_seteuid(asroot, injail)", error, 0, 0);
	if (asroot && !injail)
		expect("priv_seteuid(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_seteuid(!asroot, injail)", error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_seteuid(!asroot, !injail)", error, -1, EPERM);
}

void
priv_cred_setgid(int asroot, int injail, struct test *test)
{
	int error;

	error = setgid(GID_OTHER);
	if (asroot && injail)
		expect("priv_setgid(asroot, injail)", error, 0, 0);
	if (asroot && !injail)
		expect("priv_setgid(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_setgid(!asroot, injail)", error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_setgid(!asroot, !injail)", error, -1, EPERM);
}

void
priv_cred_setegid(int asroot, int injail, struct test *test)
{
	int error;

	error = setegid(GID_OTHER);
	if (asroot && injail)
		expect("priv_setegid(asroot, injail)", error, 0, 0);
	if (asroot && !injail)
		expect("priv_setegid(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_setegd(!asroot, injail)", error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_setegid(!asroot, !injail)", error, -1, EPERM);
}

static const gid_t	gidset[] = {GID_WHEEL, GID_OTHER};
static const int	gidset_len = sizeof(gidset) / sizeof(gid_t);

void
priv_cred_setgroups(int asroot, int injail, struct test *test)
{
	int error;

	error = setgroups(gidset_len, gidset);
	if (asroot && injail)
		expect("priv_setgroups(asroot, injail)", error, 0, 0);
	if (asroot && !injail)
		expect("priv_setgroups(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_setgroups(!asroot, injail)", error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_setgroups(!asroot, !injail)", error, -1, EPERM);
}

void
priv_cred_setreuid(int asroot, int injail, struct test *test)
{
	int error;

	error = setreuid(UID_OTHER, UID_OTHER);
	if (asroot && injail)
		expect("priv_setreuid(asroot, injail)", error, 0, 0);
	if (asroot && !injail)
		expect("priv_setreuid(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_setreuid(!asroot, injail)", error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_setreuid(!asroot, !injail)", error, -1, EPERM);
}

void
priv_cred_setregid(int asroot, int injail, struct test *test)
{
	int error;

	error = setregid(GID_OTHER, GID_OTHER);
	if (asroot && injail)
		expect("priv_setregid(asroot, injail)", error, 0, 0);
	if (asroot && !injail)
		expect("priv_setregid(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_setregid(!asroot, injail)", error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_setregid(!asroot, !injail)", error, -1, EPERM);
}

void
priv_cred_setresuid(int asroot, int injail, struct test *test)
{
	int error;

	error = setresuid(UID_OTHER, UID_OTHER, UID_OTHER);
	if (asroot && injail)
		expect("priv_setresuid(asroot, injail)", error, 0, 0);
	if (asroot && !injail)
		expect("priv_setresuid(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_setresuid(!asroot, injail)", error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_setresuid(!asroot, !injail)", error, -1, EPERM);
}

void
priv_cred_setresgid(int asroot, int injail, struct test *test)
{
	int error;

	error = setresgid(GID_OTHER, GID_OTHER, GID_OTHER);
	if (asroot && injail)
		expect("priv_setresgid(asroot, injail)", error, 0, 0);
	if (asroot && !injail)
		expect("priv_setresgid(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_setresgid(!asroot, injail)", error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_setresgid(!asroot, !injail)", error, -1, EPERM);
}

void
priv_cred_cleanup(int asroot, int injail, struct test *test)
{

}