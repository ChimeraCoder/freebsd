
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
#include "err.h"

ROKEN_LIB_FUNCTION void ROKEN_LIB_CALL
rk_warnerr(int doerrno, const char *fmt, va_list ap)
{
    int sverrno = errno;
    const char *progname = getprogname();

    if(progname != NULL){
	fprintf(stderr, "%s", progname);
	if(fmt != NULL || doerrno)
	    fprintf(stderr, ": ");
    }
    if (fmt != NULL){
	vfprintf(stderr, fmt, ap);
	if(doerrno)
	    fprintf(stderr, ": ");
    }
    if(doerrno)
	fprintf(stderr, "%s", strerror(sverrno));
    fprintf(stderr, "\n");
}