
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
/* Just a replacement, if the original malloc is not   GNU-compliant. See autoconf documentation. */

#if HAVE_CONFIG_H
#include <ldns/config.h>
#endif

void *calloc();

#if !HAVE_BZERO && HAVE_MEMSET
# define bzero(buf, bytes)	((void) memset (buf, 0, bytes))
#endif

void *
calloc(size_t num, size_t size)
{
	void *new = malloc(num * size);
	if (!new) {
		return NULL;
	}
	bzero(new, num * size);
	return new;
}