
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

#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <errno.h>

#include <roken.h>

#ifndef HAVE_MKSTEMP

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
mkstemp(char *template)
{
    int start, i;
    pid_t val;
    val = getpid();
    start = strlen(template) - 1;
    while(template[start] == 'X') {
	template[start] = '0' + val % 10;
	val /= 10;
	start--;
    }

    do{
	int fd;
	fd = open(template, O_RDWR | O_CREAT | O_EXCL, 0600);
	if(fd >= 0 || errno != EEXIST)
	    return fd;
	i = start + 1;
	do{
	    if(template[i] == 0)
		return -1;
	    template[i]++;
	    if(template[i] == '9' + 1)
		template[i] = 'a';
	    if(template[i] <= 'z')
		break;
	    template[i] = 'a';
	    i++;
	}while(1);
    }while(1);
}

#endif