
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

#include "ech_locl.h"

int ECDH_compute_key(void *out, size_t outlen, const EC_POINT *pub_key,
	EC_KEY *eckey,
	void *(*KDF)(const void *in, size_t inlen, void *out, size_t *outlen))
{
	ECDH_DATA *ecdh = ecdh_check(eckey);
	if (ecdh == NULL)
		return 0;
	return ecdh->meth->compute_key(out, outlen, pub_key, eckey, KDF);
}