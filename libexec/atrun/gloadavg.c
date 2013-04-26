
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

#ifndef lint
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#ifndef __FreeBSD__
#define _POSIX_SOURCE 1

/* System Headers */

#include <stdio.h>
#else
#include <stdlib.h>
#endif

/* Local headers */

#include "gloadavg.h"

/* Global functions */

void perr(const char *fmt, ...);

double
gloadavg(void)
/* return the current load average as a floating point number, or <0 for
 * error
 */
{
    double result;
#ifndef __FreeBSD__
    FILE *fp;
    
    if((fp=fopen(PROC_DIR "loadavg","r")) == NULL)
	result = -1.0;
    else
    {
	if(fscanf(fp,"%lf",&result) != 1)
	    result = -1.0;
	fclose(fp);
    }
#else
    if (getloadavg(&result, 1) != 1)
	    perr("error in getloadavg");
#endif
    return result;
}