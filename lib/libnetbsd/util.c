
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
#include <sys/types.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <util.h>

char *
flags_to_string(u_long flags, const char *def)
{
	char *str;

	str = fflagstostr(flags);
	if (*str == '\0') {
		free(str);
		str = strdup(def);
	}
	return (str);
}

int
string_to_flags(char **stringp, u_long *setp, u_long *clrp)
{

	return strtofflags(stringp, setp, clrp);
}