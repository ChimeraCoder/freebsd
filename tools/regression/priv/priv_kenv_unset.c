
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
 * Test setting a kernel environment variable, then try to unset it.
 */

#include <sys/types.h>

#include <err.h>
#include <errno.h>
#include <kenv.h>
#include <string.h>
#include <unistd.h>

#include "main.h"

int
priv_kenv_unset_setup(int asroot, int injail, struct test *test)
{

	if (kenv(KENV_SET, KENV_VAR_NAME, KENV_VAR_VALUE, KENV_VAR_LEN) < 0) {
		warn("priv_kenv_unset: kenv");
		return (-1);
	}
	return (0);
}

void
priv_kenv_unset(int asroot, int injail, struct test *test)
{
	int error;

	error = kenv(KENV_UNSET, KENV_VAR_NAME, NULL, 0);
	if (asroot && injail)
		expect("priv_kenv_unset(asroot, injail)", error, -1, EPERM);
	if (asroot && !injail)
		expect("priv_kenv_unset(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_kenv_unset(!asroot, injail)", error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_kenv_unset(!asroot, !injail)", error, -1, EPERM);
}

void
priv_kenv_unset_cleanup(int asroot, int injail, struct test *test)
{

	(void)kenv(KENV_UNSET, KENV_VAR_NAME, NULL, 0);
}