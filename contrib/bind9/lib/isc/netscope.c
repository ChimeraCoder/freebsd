
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

/*! \file */

#if defined(LIBC_SCCS) && !defined(lint)
static char rcsid[] =
	"$Id: netscope.c,v 1.13 2007/06/19 23:47:17 tbox Exp $";
#endif /* LIBC_SCCS and not lint */

#include <config.h>

#include <isc/string.h>
#include <isc/net.h>
#include <isc/netscope.h>
#include <isc/result.h>

isc_result_t
isc_netscope_pton(int af, char *scopename, void *addr, isc_uint32_t *zoneid) {
	char *ep;
#ifdef ISC_PLATFORM_HAVEIFNAMETOINDEX
	unsigned int ifid;
#endif
	struct in6_addr *in6;
	isc_uint32_t zone;
	isc_uint64_t llz;

	/* at this moment, we only support AF_INET6 */
	if (af != AF_INET6)
		return (ISC_R_FAILURE);

	in6 = (struct in6_addr *)addr;

	/*
	 * Basically, "names" are more stable than numeric IDs in terms of
	 * renumbering, and are more preferred.  However, since there is no
	 * standard naming convention and APIs to deal with the names.  Thus,
	 * we only handle the case of link-local addresses, for which we use
	 * interface names as link names, assuming one to one mapping between
	 * interfaces and links.
	 */
#ifdef ISC_PLATFORM_HAVEIFNAMETOINDEX
	if (IN6_IS_ADDR_LINKLOCAL(in6) &&
	    (ifid = if_nametoindex((const char *)scopename)) != 0)
		zone = (isc_uint32_t)ifid;
	else {
#endif
		llz = isc_string_touint64(scopename, &ep, 10);
		if (ep == scopename)
			return (ISC_R_FAILURE);

		/* check overflow */
		zone = (isc_uint32_t)(llz & 0xffffffffUL);
		if (zone != llz)
			return (ISC_R_FAILURE);
#ifdef ISC_PLATFORM_HAVEIFNAMETOINDEX
	}
#endif

	*zoneid = zone;
	return (ISC_R_SUCCESS);
}