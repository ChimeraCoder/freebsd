
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

#include "login_locl.h"

RCSID("$Id$");

static char *confbuf;

static int
login_conf_init(void)
{
    char *files[] = { _PATH_LOGIN_CONF, NULL };
    return cgetent(&confbuf, files, "default");
}

char *
login_conf_get_string(const char *str)
{
    char *value;
    if(login_conf_init() != 0)
	return NULL;
    if(cgetstr(confbuf, (char *)str, &value) < 0)
	return NULL;
    return value;
}