
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

#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <ulog.h>

/*
 * This setuid helper utility writes user login records to disk.
 * Unprivileged processes are not capable of writing records to utmpx,
 * but we do want to allow this for pseudo-terminals.  Because a file
 * descriptor to a pseudo-terminal master device can only be obtained by
 * processes using the pseudo-terminal, we expect such a descriptor on
 * stdin.
 *
 * It uses the real user ID of the calling process to determine the
 * username.  It does allow users to log arbitrary hostnames.
 */

static const char *
get_username(void)
{
	const struct passwd *pw;
	const char *login;
	uid_t uid;

	/*
	 * Attempt to determine the username corresponding to this login
	 * session.  First, validate the results of getlogin() against
	 * the password database.  If getlogin() returns invalid data,
	 * return an arbitrary username corresponding to this uid.
	 */
	uid = getuid();
	if ((login = getlogin()) != NULL && (pw = getpwnam(login)) != NULL &&
	    pw->pw_uid == uid)
		return (login);
	if ((pw = getpwuid(uid)) != NULL)
		return (pw->pw_name);
	return (NULL);
}

int
main(int argc, char *argv[])
{
	const char *line, *user, *host;

	/* Device line name. */
	if ((line = ptsname(STDIN_FILENO)) == NULL)
		return (EX_USAGE);

	if ((argc == 2 || argc == 3) && strcmp(argv[1], "login") == 0) {
		/* Username. */
		user = get_username();
		if (user == NULL)
			return (EX_OSERR);

		/* Hostname. */
		host = argc == 3 ? argv[2] : NULL;

		ulog_login(line, user, host);
		return (EX_OK);
	} else if (argc == 2 && strcmp(argv[1], "logout") == 0) {
		ulog_logout(line);
		return (EX_OK);
	}

	return (EX_USAGE);
}