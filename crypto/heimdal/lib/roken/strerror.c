
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
#include <string.h>
#include <errno.h>

extern int sys_nerr;
extern char *sys_errlist[];

ROKEN_LIB_FUNCTION char* ROKEN_LIB_CALL
strerror(int eno)
{
    static char emsg[1024];

    if(eno < 0 || eno >= sys_nerr)
	snprintf(emsg, sizeof(emsg), "Error %d occurred.", eno);
    else
	snprintf(emsg, sizeof(emsg), "%s", sys_errlist[eno]);

    return emsg;
}