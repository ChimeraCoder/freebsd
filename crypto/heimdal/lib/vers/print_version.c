
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

#define VERSION_HIDDEN static

#include "roken.h"

#include "version.h"

void ROKEN_LIB_FUNCTION
print_version(const char *progname)
{
    const char *package_list = heimdal_version;

    if(progname == NULL)
	progname = getprogname();

    if(*package_list == '\0')
	package_list = "no version information";
    fprintf(stderr, "%s (%s)\n", progname, package_list);
    fprintf(stderr, "Copyright 1995-2011 Kungliga Tekniska Högskolan\n");
#ifdef PACKAGE_BUGREPORT
    fprintf(stderr, "Send bug-reports to %s\n", PACKAGE_BUGREPORT);
#endif
}