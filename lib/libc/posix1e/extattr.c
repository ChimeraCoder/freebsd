
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

#include <sys/types.h>
#include <sys/extattr.h>

#include <errno.h>
#include <libutil.h>
#include <string.h>

int
extattr_namespace_to_string(int attrnamespace, char **string)
{

	switch(attrnamespace) {
	case EXTATTR_NAMESPACE_USER:
		if (string != NULL)
			*string = strdup(EXTATTR_NAMESPACE_USER_STRING);
		return (0);

	case EXTATTR_NAMESPACE_SYSTEM:
		if (string != NULL)
			*string = strdup(EXTATTR_NAMESPACE_SYSTEM_STRING);
		return (0);

	default:
		errno = EINVAL;
		return (-1);
	}
}

int
extattr_string_to_namespace(const char *string, int *attrnamespace)
{

	if (!strcmp(string, EXTATTR_NAMESPACE_USER_STRING)) {
		if (attrnamespace != NULL)
			*attrnamespace = EXTATTR_NAMESPACE_USER;
		return (0);
	} else if (!strcmp(string, EXTATTR_NAMESPACE_SYSTEM_STRING)) {
		if (attrnamespace != NULL)
			*attrnamespace = EXTATTR_NAMESPACE_SYSTEM;
		return (0);
	} else {
		errno = EINVAL;
		return (-1);
	}
}