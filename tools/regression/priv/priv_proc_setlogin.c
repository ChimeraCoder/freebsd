
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
 * Test privileges for setlogin(); first query with getlogin() so that the
 * result is a no-op, since it affects the entire login session.
 */

#include <sys/param.h>

#include <err.h>
#include <errno.h>
#include <unistd.h>

#include "main.h"

static int initialized;
static char *loginname;

int
priv_proc_setlogin_setup(int asroot, int injail, struct test *test)
{

	if (initialized)
		return (0);
	loginname = getlogin();
	if (loginname == NULL) {
		warn("priv_proc_setlogin_setup: getlogin");
		return (-1);
	}
	initialized = 1;
	return (0);
}

void
priv_proc_setlogin(int asroot, int injail, struct test *test)
{
	int error;

	error = setlogin(loginname);
	if (asroot && injail)
		expect("priv_proc_setlogin(asroot, injail)", error, 0, 0);
	if (asroot && !injail)
		expect("priv_proc_setlogin(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_proc_setlogin(!sroot, injail)", error, -1,
		    EPERM);
	if (!asroot && !injail)
		expect("priv_proc_setlogin(!asroot, !injail)", error, -1,
		    EPERM);
}

void
priv_proc_setlogin_cleanup(int asroot, int injail, struct test *test)
{

	if (initialized)
		(void)setlogin(loginname);
}