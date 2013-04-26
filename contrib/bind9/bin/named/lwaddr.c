
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

/* $Id: lwaddr.c,v 1.10 2008/01/11 23:46:56 tbox Exp $ */

/*! \file */

#include <config.h>

#include <string.h>

#include <isc/result.h>
#include <isc/netaddr.h>
#include <isc/sockaddr.h>

#include <lwres/lwres.h>

#include <named/lwaddr.h>

/*%
 * Convert addresses from lwres to isc format.
 */
isc_result_t
lwaddr_netaddr_fromlwresaddr(isc_netaddr_t *na, lwres_addr_t *la) {
	if (la->family != LWRES_ADDRTYPE_V4 && la->family != LWRES_ADDRTYPE_V6)
		return (ISC_R_FAMILYNOSUPPORT);

	if (la->family == LWRES_ADDRTYPE_V4) {
		struct in_addr ina;
		memcpy(&ina.s_addr, la->address, 4);
		isc_netaddr_fromin(na, &ina);
	} else {
		struct in6_addr ina6;
		memcpy(&ina6.s6_addr, la->address, 16);
		isc_netaddr_fromin6(na, &ina6);
	}
	return (ISC_R_SUCCESS);
}

isc_result_t
lwaddr_sockaddr_fromlwresaddr(isc_sockaddr_t *sa, lwres_addr_t *la,
			      in_port_t port)
{
	isc_netaddr_t na;
	isc_result_t result;

	result = lwaddr_netaddr_fromlwresaddr(&na, la);
	if (result != ISC_R_SUCCESS)
		return (result);
	isc_sockaddr_fromnetaddr(sa, &na, port);
	return (ISC_R_SUCCESS);
}

/*%
 * Convert addresses from isc to lwres format.
 */

isc_result_t
lwaddr_lwresaddr_fromnetaddr(lwres_addr_t *la, isc_netaddr_t *na) {
	if (na->family != AF_INET && na->family != AF_INET6)
		return (ISC_R_FAMILYNOSUPPORT);

	if (na->family == AF_INET) {
		la->family = LWRES_ADDRTYPE_V4;
		la->length = 4;
		memcpy(la->address, &na->type.in, 4);
	} else {
		la->family = LWRES_ADDRTYPE_V6;
		la->length = 16;
		memcpy(la->address, &na->type.in6, 16);
	}
	return (ISC_R_SUCCESS);
}

isc_result_t
lwaddr_lwresaddr_fromsockaddr(lwres_addr_t *la, isc_sockaddr_t *sa) {
	isc_netaddr_t na;
	isc_netaddr_fromsockaddr(&na, sa);
	return (lwaddr_lwresaddr_fromnetaddr(la, &na));
}