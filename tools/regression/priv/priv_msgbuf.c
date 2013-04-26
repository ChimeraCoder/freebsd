
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
 * Confirm that when security.bsd.unprivileged_read_msgbuf is set to 0,
 * privilege is required to read the kernel message buffer.
 */

#include <sys/types.h>
#include <sys/sysctl.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>

#include "main.h"

#define	MSGBUF_CONTROL_NAME	"security.bsd.unprivileged_read_msgbuf"
#define	MSGBUF_NAME		"kern.msgbuf"

/*
 * We must query and save the original value, then restore it when done.
 */
static int unprivileged_read_msgbuf;
static int unprivileged_read_msgbuf_initialized;

int
priv_msgbuf_privonly_setup(int asroot, int injail, struct test *test)
{
	size_t len;
	int newval;

	/*
	 * Separately query and set to make debugging easier.
	 */
	len = sizeof(unprivileged_read_msgbuf);
	if (sysctlbyname(MSGBUF_CONTROL_NAME, &unprivileged_read_msgbuf,
	    &len, NULL, 0) < 0) {
		warn("priv_msgbuf_privonly_setup: sysctlbyname query");
		return (-1);
	}
	newval = 0;
	if (sysctlbyname(MSGBUF_CONTROL_NAME, NULL, NULL, &newval,
	    sizeof(newval)) < 0) {
		warn("priv_msgbuf_privonly_setup: sysctlbyname set");
		return (-1);
	}
	unprivileged_read_msgbuf_initialized = 1;
	return (0);
}

void
priv_msgbuf_privonly(int asroot, int injail, struct test *test)
{
	size_t len;
	int error;

	error = sysctlbyname(MSGBUF_NAME, NULL, &len, NULL, 0);
	if (asroot && injail)
		expect("priv_msgbuf_privonly(asroot, injail)", error, -1,
		    EPERM);
	if (asroot && !injail)
		expect("priv_msgbuf_privonly(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_msgbuf_privonly(!asroot, injail)", error, -1,
		    EPERM);
	if (!asroot && !injail)
		expect("priv_msgbuf_privonly(!asroot, !injail)", error, -1,
		    EPERM);
}

int
priv_msgbuf_unprivok_setup(int asroot, int injail, struct test *test)
{
	size_t len;
	int newval;

	/*
	 * Separately query and set to make debugging easier.
	 */
	len = sizeof(unprivileged_read_msgbuf);
	if (sysctlbyname(MSGBUF_CONTROL_NAME, &unprivileged_read_msgbuf, &len,
	    NULL, 0) < 0) {
		warn("priv_msgbuf_unprivok_setup: sysctlbyname query");
		return (-1);
	}
	newval = 1;
	if (sysctlbyname(MSGBUF_CONTROL_NAME, NULL, NULL, &newval,
	    sizeof(newval)) < 0) {
		warn("priv_msgbuf_unprivok_setup: sysctlbyname set");
		return (-1);
	}
	unprivileged_read_msgbuf_initialized = 1;
	return (0);
}

void
priv_msgbuf_unprivok(int asroot, int injail, struct test *test)
{
	size_t len;
	int error;

	error = sysctlbyname(MSGBUF_NAME, NULL, &len, NULL, 0);
	if (asroot && injail)
		expect("priv_msgbuf_unprivok(asroot, injail)", error, 0, 0);
	if (asroot && !injail)
		expect("priv_msgbuf_unprivok(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_msgbuf_unprivok(!asroot, injail)", error, 0, 0);
	if (!asroot && !injail)
		expect("priv_msgbuf_unprivok(!asroot, !injail)", error, 0, 0);
}

void
priv_msgbuf_cleanup(int asroot, int injail, struct test *test)
{

	if (unprivileged_read_msgbuf_initialized) {
		(void)sysctlbyname(MSGBUF_NAME, NULL, NULL,
		    &unprivileged_read_msgbuf,
		    sizeof(unprivileged_read_msgbuf));
		unprivileged_read_msgbuf_initialized = 0;
	}
}