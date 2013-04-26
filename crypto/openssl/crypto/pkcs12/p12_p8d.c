
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

#include <stdio.h>
#include "cryptlib.h"
#include <openssl/pkcs12.h>

PKCS8_PRIV_KEY_INFO *PKCS8_decrypt(X509_SIG *p8, const char *pass, int passlen)
{
	return PKCS12_item_decrypt_d2i(p8->algor, ASN1_ITEM_rptr(PKCS8_PRIV_KEY_INFO), pass,
					passlen, p8->digest, 1);
}