
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

#ifndef HAVE_GETHOSTNAME

#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif

/*
 * Return the local host's name in "name", up to "namelen" characters.
 * "name" will be null-terminated if "namelen" is big enough.
 * The return code is 0 on success, -1 on failure.  (The calling
 * interface is identical to gethostname(2).)
 */

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
gethostname(char *name, int namelen)
{
#if defined(HAVE_UNAME)
    {
	struct utsname utsname;
	int ret;

	ret = uname (&utsname);
	if (ret < 0)
	    return ret;
	strlcpy (name, utsname.nodename, namelen);
	return 0;
    }
#else
    strlcpy (name, "some.random.host", namelen);
    return 0;
#endif
}

#endif /* GETHOSTNAME */