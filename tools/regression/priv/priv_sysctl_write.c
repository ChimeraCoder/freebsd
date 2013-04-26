
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
 * Two privileges exist for writing sysctls -- one for sysctls writable only
 * outside of jail (PRIV_SYSCTL_WRITE) and one for those also writable inside
 * jail (PRIV_SYSCTL_WRITEJAIL).
 *
 * Test the prior by attempting to write to kern.domainname, and the latter
 * by attempting to write to kern.hostname.
 */

#include <sys/types.h>
#include <sys/sysctl.h>

#include <err.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "main.h"

#define KERN_HOSTNAME_STRING	"kern.hostname"
#define	KERN_DOMAINNAME_STRING	"kern.domainname"

static char stored_hostname[1024];
static char stored_domainname[1024];

int
priv_sysctl_write_setup(int asroot, int injail, struct test *test)
{
	size_t len;
	int error;

	len = sizeof(stored_hostname);
	error = sysctlbyname(KERN_HOSTNAME_STRING, stored_hostname, &len,
	    NULL, 0);
	if (error) {
		warn("priv_sysctl_write_setup: sysctlbyname(\"%s\")",
		    KERN_HOSTNAME_STRING);
		return (-1);
	}

	len = sizeof(stored_hostname);
	error = sysctlbyname(KERN_DOMAINNAME_STRING, stored_domainname, &len,
	    NULL, 0);
	if (error) {
		warn("priv_sysctl_write_setup: sysctlbyname(\"%s\")",
		    KERN_DOMAINNAME_STRING);
		return (-1);
	}

	return (0);
}

void
priv_sysctl_write(int asroot, int injail, struct test *test)
{
	int error;

	error = sysctlbyname(KERN_DOMAINNAME_STRING, NULL, NULL,
	    stored_domainname, strlen(stored_domainname));
	if (asroot && injail)
		expect("priv_sysctl_write(asroot, injail)", error, -1,
		    EPERM);
	if (asroot && !injail)
		expect("priv_sysctl_write(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_sysctl_write(!asroot, injail)", error, -1,
		    EPERM);
	if (!asroot && !injail)
		expect("priv_sysctl_write(!asroot, !injail)", error, -1,
		    EPERM);
}

void
priv_sysctl_writejail(int asroot, int injail, struct test *test)
{
	int error;

	error = sysctlbyname(KERN_HOSTNAME_STRING, NULL, NULL,
	    stored_hostname, strlen(stored_hostname));
	if (asroot && injail)
		expect("priv_sysctl_writejail(asroot, injail)", error, 0, 0);
	if (asroot && !injail)
		expect("priv_sysctl_writejail(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_sysctl_writejail(!asroot, injail)", error, -1,
		    EPERM);
	if (!asroot && !injail)
		expect("priv_sysctl_writejail(!asroot, !injail)", error, -1,
		    EPERM);
}

void
priv_sysctl_write_cleanup(int asroot, int injail, struct test *test)
{

}