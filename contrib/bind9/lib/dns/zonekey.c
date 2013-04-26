
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

/* $Id: zonekey.c,v 1.9 2007/06/19 23:47:16 tbox Exp $ */

/*! \file */

#include <config.h>

#include <isc/result.h>
#include <isc/types.h>
#include <isc/util.h>

#include <dns/keyvalues.h>
#include <dns/rdata.h>
#include <dns/rdatastruct.h>
#include <dns/types.h>
#include <dns/zonekey.h>

isc_boolean_t
dns_zonekey_iszonekey(dns_rdata_t *keyrdata) {
	isc_result_t result;
	dns_rdata_dnskey_t key;
	isc_boolean_t iszonekey = ISC_TRUE;

	REQUIRE(keyrdata != NULL);

	result = dns_rdata_tostruct(keyrdata, &key, NULL);
	if (result != ISC_R_SUCCESS)
		return (ISC_FALSE);

	if ((key.flags & DNS_KEYTYPE_NOAUTH) != 0)
		iszonekey = ISC_FALSE;
	if ((key.flags & DNS_KEYFLAG_OWNERMASK) != DNS_KEYOWNER_ZONE)
		iszonekey = ISC_FALSE;
	if (key.protocol != DNS_KEYPROTO_DNSSEC &&
	key.protocol != DNS_KEYPROTO_ANY)
		iszonekey = ISC_FALSE;
	
	return (iszonekey);
}