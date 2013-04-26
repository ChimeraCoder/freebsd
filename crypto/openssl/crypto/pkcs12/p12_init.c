
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

/* Initialise a PKCS12 structure to take data */

PKCS12 *PKCS12_init(int mode)
{
	PKCS12 *pkcs12;
	if (!(pkcs12 = PKCS12_new())) {
		PKCS12err(PKCS12_F_PKCS12_INIT,ERR_R_MALLOC_FAILURE);
		return NULL;
	}
	ASN1_INTEGER_set(pkcs12->version, 3);
	pkcs12->authsafes->type = OBJ_nid2obj(mode);
	switch (mode) {
		case NID_pkcs7_data:
			if (!(pkcs12->authsafes->d.data =
				 M_ASN1_OCTET_STRING_new())) {
			PKCS12err(PKCS12_F_PKCS12_INIT,ERR_R_MALLOC_FAILURE);
			goto err;
		}
		break;
		default:
			PKCS12err(PKCS12_F_PKCS12_INIT,
				PKCS12_R_UNSUPPORTED_PKCS12_MODE);
			goto err;
	}
		
	return pkcs12;
err:
	if (pkcs12 != NULL) PKCS12_free(pkcs12);
	return NULL;
}