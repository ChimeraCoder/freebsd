
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

/*
 * memmove for systems that doesn't have it
 */

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

ROKEN_LIB_FUNCTION void* ROKEN_LIB_CALL
memmove(void *s1, const void *s2, size_t n)
{
  char *s=(char*)s2, *d=(char*)s1;

  if(d > s){
    s+=n-1;
    d+=n-1;
    while(n){
      *d--=*s--;
      n--;
    }
  }else if(d < s)
    while(n){
      *d++=*s++;
      n--;
    }
  return s1;
}