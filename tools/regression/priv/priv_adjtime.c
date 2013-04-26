
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
 * Confirm that privilege is required to invoke adjtime(); first query, then
 * try setting with and without privilege.  Hopefully this will not disturb
 * system time too much.
 */

#include <sys/time.h>

#include <err.h>
#include <errno.h>
#include <unistd.h>

#include "main.h"

static int		initialized;
static struct timeval	query_tv;

int
priv_adjtime_setup(int asroot, int injail, struct test *test)
{

	if (initialized)
		return (0);
	if (adjtime(NULL, &query_tv) < 0) {
		warn("priv_adjtime_setup: adjtime(NULL)");
		return (-1);
	}
	initialized = 1;
	return (0);
}

void
priv_adjtime_set(int asroot, int injail, struct test *test)
{
	int error;

	error = adjtime(&query_tv, NULL);
	if (asroot && injail)
		expect("priv_adjtime(asroot, injail)", error, -1, EPERM);
	if (asroot && !injail)
		expect("priv_adjtime(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_adjtime(!asroot, injail)", error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_adjtime(!asroot, !injail)", error, -1, EPERM);
}

void
priv_adjtime_cleanup(int asroot, int injail, struct test *test)
{

}