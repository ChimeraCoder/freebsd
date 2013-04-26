
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

#include <config.h>

#include "roken.h"

/*
 * Try to return what should be considered the default username or
 * NULL if we can't guess at all.
 */

ROKEN_LIB_FUNCTION const char * ROKEN_LIB_CALL
get_default_username (void)
{
    const char *user;

    user = getenv ("USER");
    if (user == NULL)
	user = getenv ("LOGNAME");
    if (user == NULL)
	user = getenv ("USERNAME");

#if defined(HAVE_GETLOGIN) && !defined(POSIX_GETLOGIN)
    if (user == NULL) {
	user = (const char *)getlogin ();
	if (user != NULL)
	    return user;
    }
#endif
#ifdef HAVE_PWD_H
    {
	uid_t uid = getuid ();
	struct passwd *pwd;

	if (user != NULL) {
	    pwd = k_getpwnam (user);
	    if (pwd != NULL && pwd->pw_uid == uid)
		return user;
	}
	pwd = k_getpwuid (uid);
	if (pwd != NULL)
	    return pwd->pw_name;
    }
#endif
#ifdef _WIN32
    /* TODO: We can call GetUserNameEx() and figure out a
       username. However, callers do not free the return value of this
       function. */
#endif

    return user;
}