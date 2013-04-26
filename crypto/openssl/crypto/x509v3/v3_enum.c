
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
#include <openssl/x509v3.h>

static ENUMERATED_NAMES crl_reasons[] = {
{CRL_REASON_UNSPECIFIED, 	 "Unspecified", "unspecified"},
{CRL_REASON_KEY_COMPROMISE,	 "Key Compromise", "keyCompromise"},
{CRL_REASON_CA_COMPROMISE,	 "CA Compromise", "CACompromise"},
{CRL_REASON_AFFILIATION_CHANGED, "Affiliation Changed", "affiliationChanged"},
{CRL_REASON_SUPERSEDED, 	 "Superseded", "superseded"},
{CRL_REASON_CESSATION_OF_OPERATION,
			"Cessation Of Operation", "cessationOfOperation"},
{CRL_REASON_CERTIFICATE_HOLD,	 "Certificate Hold", "certificateHold"},
{CRL_REASON_REMOVE_FROM_CRL,	 "Remove From CRL", "removeFromCRL"},
{CRL_REASON_PRIVILEGE_WITHDRAWN, "Privilege Withdrawn", "privilegeWithdrawn"},
{CRL_REASON_AA_COMPROMISE,	 "AA Compromise", "AACompromise"},
{-1, NULL, NULL}
};

const X509V3_EXT_METHOD v3_crl_reason = { 
NID_crl_reason, 0, ASN1_ITEM_ref(ASN1_ENUMERATED),
0,0,0,0,
(X509V3_EXT_I2S)i2s_ASN1_ENUMERATED_TABLE,
0,
0,0,0,0,
crl_reasons};


char *i2s_ASN1_ENUMERATED_TABLE(X509V3_EXT_METHOD *method,
	     ASN1_ENUMERATED *e)
{
	ENUMERATED_NAMES *enam;
	long strval;
	strval = ASN1_ENUMERATED_get(e);
	for(enam = method->usr_data; enam->lname; enam++) {
		if(strval == enam->bitnum) return BUF_strdup(enam->lname);
	}
	return i2s_ASN1_ENUMERATED(method, e);
}