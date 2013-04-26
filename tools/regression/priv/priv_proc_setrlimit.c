
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

/*-
 * Test that raising current resource limits above hard resource limits
 * requires privilege.  We test three cases:
 *
 * - Raise the current above the maximum (privileged).
 * - Raise the current to the maximum (unprivileged).
 * - Raise the maximum (privileged).
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <err.h>
#include <errno.h>
#include <unistd.h>

#include "main.h"

static int initialized;
static struct rlimit rl_base;
static struct rlimit rl_lowered;

int
priv_proc_setrlimit_setup(int asroot, int injail, struct test *test)
{

	if (getrlimit(RLIMIT_DATA, &rl_base) < 0) {
		warn("priv_proc_setrlimit_setup: getrlimit");
		return (-1);
	}

	/*
	 * Must lower current and limit to make sure there's room to try to
	 * raise them during tests.  Set current lower than max so we can
	 * raise it later also.
	 */
	rl_lowered = rl_base;
	rl_lowered.rlim_cur -= 20;
	rl_lowered.rlim_max -= 10;
	if (setrlimit(RLIMIT_DATA, &rl_lowered) < 0) {
		warn("priv_proc_setrlimit_setup: setrlimit");
		return (-1);
	}
	initialized = 1;
	return (0);
}

/*
 * Try increasing the maximum limits on the process, which requires
 * privilege.
 */
void
priv_proc_setrlimit_raisemax(int asroot, int injail, struct test *test)
{
	struct rlimit rl;
	int error;

	rl = rl_lowered;
	rl.rlim_max = rl_base.rlim_max;
	error = setrlimit(RLIMIT_DATA, &rl);
	if (asroot && injail)
		expect("priv_proc_setrlimit_raisemax(asroot, injail)", error,
		    0, 0);
	if (asroot && !injail)
		expect("priv_proc_setrlimit_raisemax(asroot, !injail)",
		    error, 0, 0);
	if (!asroot && injail)
		expect("priv_proc_setrlimit_raisemax(!asroot, injail)",
		    error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_proc_setrlimit_raisemax(!asroot, !injail)",
		    error, -1, EPERM);
}

/*
 * Try setting the current limit to the current maximum, which is allowed
 * without privilege.
 */
void
priv_proc_setrlimit_raisecur_nopriv(int asroot, int injail,
    struct test *test)
{
	struct rlimit rl;
	int error;

	rl = rl_lowered;
	rl.rlim_cur = rl.rlim_max;
	error = setrlimit(RLIMIT_DATA, &rl);
	if (asroot && injail)
		expect("priv_proc_setrlimit_raiscur_nopriv(asroot, injail)",
		    error, 0, 0);
	if (asroot && !injail)
		expect("priv_proc_setrlimit_raisecur_nopriv(asroot, !injail)",
		    error, 0, 0);
	if (!asroot && injail)
		expect("priv_proc_setrlimit_raisecur_nopriv(!asroot, injail)",
		    error, 0, 0);
	if (!asroot && !injail)
		expect("priv_proc_setrlimit_raisecur_nopriv(!asroot, !injail)",
		    error, 0, 0);
}

/*
 * Try raising the current limits above the maximum, which requires
 * privilege.
 */
void
priv_proc_setrlimit_raisecur(int asroot, int injail, struct test *test)
{
	struct rlimit rl;
	int error;

	rl = rl_lowered;
	rl.rlim_cur = rl.rlim_max + 10;
	error = setrlimit(RLIMIT_DATA, &rl);
	if (asroot && injail)
		expect("priv_proc_setrlimit_raisecur(asroot, injail)", error,
		    0, 0);
	if (asroot && !injail)
		expect("priv_proc_setrlimit_raisecur(asroot, !injail)",
		    error, 0, 0);
	if (!asroot && injail)
		expect("priv_proc_setrlimit_raisecur(!asroot, injail)",
		    error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_proc_setrlimit_raisecur(!asroot, !injail)",
		    error, -1, EPERM);
}

void
priv_proc_setrlimit_cleanup(int asroot, int injail, struct test *test)
{

	if (initialized)
		(void)setrlimit(RLIMIT_DATA, &rl_base);
	initialized = 0;
}