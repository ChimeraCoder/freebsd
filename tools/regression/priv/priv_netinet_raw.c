
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
 * Confirm that privilege is required to open a raw IP socket, and that this
 * is not allowed in jail.
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "main.h"

int
priv_netinet_raw_setup(int asroot, int injail, struct test *test)
{

	return (0);
}

void
priv_netinet_raw(int asroot, int injail, struct test *test)
{
	int error, fd;

	fd = socket(PF_INET, SOCK_RAW, 0);
	if (fd < 0)
		error = -1;
	else
		error = 0;
	if (asroot && injail)
		expect("priv_netinet_raw(asroot, injail)", error, -1, EPERM);
	if (asroot && !injail)
		expect("priv_netinet_raw(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_netinet_raw(!asroot, injail)", error, -1,
		    EPERM);
	if (!asroot && !injail)
		expect("priv_netinet_raw(!asroot, !injail)", error,
		    -1, EPERM);
	if (fd >= 0)
		(void)close(fd);
}

void
priv_netinet_raw_cleanup(int asroot, int injail, struct test *test)
{

}