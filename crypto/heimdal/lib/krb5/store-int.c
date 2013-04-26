
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

#include "krb5_locl.h"

KRB5_LIB_FUNCTION krb5_ssize_t KRB5_LIB_CALL
_krb5_put_int(void *buffer, unsigned long value, size_t size)
{
    unsigned char *p = buffer;
    int i;
    for (i = size - 1; i >= 0; i--) {
	p[i] = value & 0xff;
	value >>= 8;
    }
    return size;
}

KRB5_LIB_FUNCTION krb5_ssize_t KRB5_LIB_CALL
_krb5_get_int(void *buffer, unsigned long *value, size_t size)
{
    unsigned char *p = buffer;
    unsigned long v = 0;
    size_t i;
    for (i = 0; i < size; i++)
	v = (v << 8) + p[i];
    *value = v;
    return size;
}