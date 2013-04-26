
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

#include <string.h>
#include <openssl/crypto.h>

unsigned char cleanse_ctr = 0;

void OPENSSL_cleanse(void *ptr, size_t len)
	{
	unsigned char *p = ptr;
	size_t loop = len, ctr = cleanse_ctr;
	while(loop--)
		{
		*(p++) = (unsigned char)ctr;
		ctr += (17 + ((size_t)p & 0xF));
		}
	p=memchr(ptr, (unsigned char)ctr, len);
	if(p)
		ctr += (63 + (size_t)p);
	cleanse_ctr = (unsigned char)ctr;
	}