
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
 * Test that configuring accounting requires privilege.  We test four cases
 * across {!jail, jail}:
 *
 * priv_acct_enable - enable accounting from a disabled state
 * priv_acct_disable - disable accounting from an enabled state
 * priv_acct_rotate - rotate the accounting file
 * priv_acct_noopdisable - disable accounting when already disabled
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysctl.h>

#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "main.h"

#define	SYSCTL_NAME	"kern.acct_configured"

/*
 * Actual filenames used across all of the tests.
 */
static int	fpath1_initialized;
static char	fpath1[1024];
static int	fpath2_initialized;
static char	fpath2[1024];

int
priv_acct_setup(int asroot, int injail, struct test *test)
{
	size_t len;
	int i;

	len = sizeof(i);
	if (sysctlbyname(SYSCTL_NAME, &i, &len, NULL, 0) < 0) {
		warn("priv_acct_setup: sysctlbyname(%s)", SYSCTL_NAME);
		return (-1);
	}
	if (i != 0) {
		warnx("sysctlbyname(%s) indicates accounting configured",
		    SYSCTL_NAME);
		return (-1);
	}
	setup_file("priv_acct_setup: fpath1", fpath1, 0, 0, 0666);
	fpath1_initialized = 1;
	setup_file("priv_acct_setup: fpath2", fpath2, 0, 0, 0666);
	fpath2_initialized = 1;

	if (test->t_test_func == priv_acct_enable ||
	    test->t_test_func == priv_acct_noopdisable) {
		if (acct(NULL) != 0) {
			warn("priv_acct_setup: acct(NULL)");
			return (-1);
		}
	} else if (test->t_test_func == priv_acct_disable ||
	     test->t_test_func == priv_acct_rotate) {
		if (acct(fpath1) != 0) {
			warn("priv_acct_setup: acct(\"%s\")", fpath1);
			return (-1);
		}
	}
	return (0);
}

void
priv_acct_cleanup(int asroot, int injail, struct test *test)
{

	(void)acct(NULL);
	if (fpath1_initialized) {
		(void)unlink(fpath1);
		fpath1_initialized = 0;
	}
	if (fpath2_initialized) {
		(void)unlink(fpath2);
		fpath2_initialized = 0;
	}
}

void
priv_acct_enable(int asroot, int injail, struct test *test)
{
	int error;

	error = acct(fpath1);
	if (asroot && injail)
		expect("priv_acct_enable(root, jail)", error, -1, EPERM);
	if (asroot && !injail)
		expect("priv_acct_enable(root, !jail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_acct_enable(!root, jail)", error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_acct_enable(!root, !jail)", error, -1, EPERM);
}

void
priv_acct_disable(int asroot, int injail, struct test *test)
{
	int error;

	error = acct(NULL);
	if (asroot && injail)
		expect("priv_acct_disable(root, jail)", error, -1, EPERM);
	if (asroot && !injail)
		expect("priv_acct_disable(root, !jail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_acct_disable(!root, jail)", error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_acct_disable(!root, !jail)", error, -1, EPERM);
}

void
priv_acct_rotate(int asroot, int injail, struct test *test)
{
	int error;

	error = acct(fpath2);
	if (asroot && injail)
		expect("priv_acct_rotate(root, jail)", error, -1, EPERM);
	if (asroot && !injail)
		expect("priv_acct_rotate(root, !jail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_acct_rotate(!root, jail)", error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_acct_rotate(!root, !jail)", error, -1, EPERM);
}

void
priv_acct_noopdisable(int asroot, int injail, struct test *test)
{
	int error;

	error = acct(NULL);
	if (asroot && injail)
		expect("priv_acct_noopdisable(root, jail)", error, -1, EPERM);
	if (asroot && !injail)
		expect("priv_acct_noopdisable(root, !jail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_acct_noopdisable(!root, jail)", error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_acct_noopdisable(!root, !jail)", error, -1, EPERM);
}