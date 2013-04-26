
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
/* Just a replacement, if the original malloc is not   GNU-compliant. Based on malloc.c */

#if HAVE_CONFIG_H
#include <ldns/config.h>
#endif
#undef realloc

#include <sys/types.h>

void *realloc (void*, size_t);
void *malloc (size_t);

/* Changes allocation to new sizes, copies over old data.
 * if oldptr is NULL, does a malloc.
 * if size is zero, allocate 1-byte block....
 *   (does not return NULL and free block)
 */

void *
rpl_realloc (void* ptr, size_t n)
{
  if (n == 0)
    n = 1;
  if(ptr == 0) {
    return malloc(n);
  }
  return realloc(ptr, n);
}