
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

#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif
#include "roken.h"

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
unix_verify_user(char *user, char *password)
{
    struct passwd *pw;

    pw = k_getpwnam(user);
    if(pw == NULL)
	return -1;
    if(strlen(pw->pw_passwd) == 0 && strlen(password) == 0)
	return 0;
    if(strcmp(crypt(password, pw->pw_passwd), pw->pw_passwd) == 0)
        return 0;
    return -1;
}