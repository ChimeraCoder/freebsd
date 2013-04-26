
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
#include <stdlib.h>
#include <openssl/asn1t.h>
#include <openssl/x509.h>
#include <openssl/objects.h>

static int nsseq_cb(int operation, ASN1_VALUE **pval, const ASN1_ITEM *it,
							void *exarg)
{
	if(operation == ASN1_OP_NEW_POST) {
		NETSCAPE_CERT_SEQUENCE *nsseq;
		nsseq = (NETSCAPE_CERT_SEQUENCE *)*pval;
		nsseq->type = OBJ_nid2obj(NID_netscape_cert_sequence);
	}
	return 1;
}

/* Netscape certificate sequence structure */

ASN1_SEQUENCE_cb(NETSCAPE_CERT_SEQUENCE, nsseq_cb) = {
	ASN1_SIMPLE(NETSCAPE_CERT_SEQUENCE, type, ASN1_OBJECT),
	ASN1_EXP_SEQUENCE_OF_OPT(NETSCAPE_CERT_SEQUENCE, certs, X509, 0)
} ASN1_SEQUENCE_END_cb(NETSCAPE_CERT_SEQUENCE, NETSCAPE_CERT_SEQUENCE)

IMPLEMENT_ASN1_FUNCTIONS(NETSCAPE_CERT_SEQUENCE)