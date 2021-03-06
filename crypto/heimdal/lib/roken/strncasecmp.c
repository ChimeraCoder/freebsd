
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
#include <ctype.h>
#include <stddef.h>

#ifndef HAVE_STRNCASECMP

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
strncasecmp(const char *s1, const char *s2, size_t n)
{
    while(n > 0
	  && toupper((unsigned char)*s1) == toupper((unsigned char)*s2))
    {
	if(*s1 == '\0')
	    return 0;
	s1++;
	s2++;
	n--;
    }
    if(n == 0)
	return 0;
    return toupper((unsigned char)*s1) - toupper((unsigned char)*s2);
}

#endif