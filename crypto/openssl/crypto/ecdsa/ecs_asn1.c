
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

#include "ecs_locl.h"
#include <openssl/err.h>
#include <openssl/asn1t.h>

ASN1_SEQUENCE(ECDSA_SIG) = {
	ASN1_SIMPLE(ECDSA_SIG, r, CBIGNUM),
	ASN1_SIMPLE(ECDSA_SIG, s, CBIGNUM)
} ASN1_SEQUENCE_END(ECDSA_SIG)

DECLARE_ASN1_FUNCTIONS_const(ECDSA_SIG)
DECLARE_ASN1_ENCODE_FUNCTIONS_const(ECDSA_SIG, ECDSA_SIG)
IMPLEMENT_ASN1_FUNCTIONS_const(ECDSA_SIG)