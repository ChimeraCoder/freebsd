
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <stdio.h>
#include <syslog.h>
#include <unistd.h>

#define	MESSAGE	"This account is currently not available.\n"

int
main(__unused int argc, __unused char *argv[])
{
	const char *user, *tt;

	if ((tt = ttyname(0)) == NULL)
		tt = "UNKNOWN";
	if ((user = getlogin()) == NULL)
		user = "UNKNOWN";
	openlog("nologin", LOG_CONS, LOG_AUTH);
	syslog(LOG_CRIT, "Attempted login by %s on %s", user, tt);
	closelog();

	printf("%s", MESSAGE);
	return 1;
}