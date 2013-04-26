
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

/* $Id: rdatasetiter.c,v 1.16 2007/06/19 23:47:16 tbox Exp $ */

/*! \file */

#include <config.h>

#include <stddef.h>

#include <isc/util.h>

#include <dns/rdataset.h>
#include <dns/rdatasetiter.h>

void
dns_rdatasetiter_destroy(dns_rdatasetiter_t **iteratorp) {
	/*
	 * Destroy '*iteratorp'.
	 */

	REQUIRE(iteratorp != NULL);
	REQUIRE(DNS_RDATASETITER_VALID(*iteratorp));

	(*iteratorp)->methods->destroy(iteratorp);

	ENSURE(*iteratorp == NULL);
}

isc_result_t
dns_rdatasetiter_first(dns_rdatasetiter_t *iterator) {
	/*
	 * Move the rdataset cursor to the first rdataset at the node (if any).
	 */

	REQUIRE(DNS_RDATASETITER_VALID(iterator));

	return (iterator->methods->first(iterator));
}

isc_result_t
dns_rdatasetiter_next(dns_rdatasetiter_t *iterator) {
	/*
	 * Move the rdataset cursor to the next rdataset at the node (if any).
	 */

	REQUIRE(DNS_RDATASETITER_VALID(iterator));

	return (iterator->methods->next(iterator));
}

void
dns_rdatasetiter_current(dns_rdatasetiter_t *iterator,
			 dns_rdataset_t *rdataset)
{
	/*
	 * Return the current rdataset.
	 */

	REQUIRE(DNS_RDATASETITER_VALID(iterator));
	REQUIRE(DNS_RDATASET_VALID(rdataset));
	REQUIRE(! dns_rdataset_isassociated(rdataset));

	iterator->methods->current(iterator, rdataset);
}