
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

#include <openssl/des.h>
#include <openssl/rand.h>

int DES_random_key(DES_cblock *ret)
	{
	do
		{
		if (RAND_bytes((unsigned char *)ret, sizeof(DES_cblock)) != 1)
			return (0);
		} while (DES_is_weak_key(ret));
	DES_set_odd_parity(ret);
	return (1);
	}