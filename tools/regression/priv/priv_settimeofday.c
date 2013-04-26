
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
 * Confirm that privilege is required to invoke settimeofday().  So as not
 * to mess up the clock too much, first query the time, then immediately set
 * it.
 */

#include <sys/time.h>

#include <err.h>
#include <errno.h>
#include <unistd.h>

#include "main.h"

static struct timeval	now;

int
priv_settimeofday_setup(int asroot, int injail, struct test *test)
{

	if (gettimeofday(&now, NULL) < 0) {
		warn("priv_settimeofday_setup: gettimeofday");
		return (-1);
	}
	return (0);
}

void
priv_settimeofday(int asroot, int injail, struct test *test)
{
	int error;

	error = settimeofday(&now, NULL);
	if (asroot && injail)
		expect("priv_settimeofday(asroot, injail)", error, -1,
		    EPERM);
	if (asroot && !injail)
		expect("priv_settimeofday(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_settimeofday(!asroot, injail)", error, -1,
		    EPERM);
	if (!asroot && !injail)
		expect("priv_settimeofday(!asroot, !injail)", error, -1,
		    EPERM);
}

void
priv_settimeofday_cleanup(int asroot, int injail, struct test *test)
{

}