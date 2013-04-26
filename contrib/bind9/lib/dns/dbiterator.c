
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

/* $Id: dbiterator.c,v 1.18 2007/06/19 23:47:16 tbox Exp $ */

/*! \file */

#include <config.h>

#include <isc/util.h>

#include <dns/dbiterator.h>
#include <dns/name.h>

void
dns_dbiterator_destroy(dns_dbiterator_t **iteratorp) {
	/*
	 * Destroy '*iteratorp'.
	 */

	REQUIRE(iteratorp != NULL);
	REQUIRE(DNS_DBITERATOR_VALID(*iteratorp));

	(*iteratorp)->methods->destroy(iteratorp);

	ENSURE(*iteratorp == NULL);
}

isc_result_t
dns_dbiterator_first(dns_dbiterator_t *iterator) {
	/*
	 * Move the node cursor to the first node in the database (if any).
	 */

	REQUIRE(DNS_DBITERATOR_VALID(iterator));

	return (iterator->methods->first(iterator));
}

isc_result_t
dns_dbiterator_last(dns_dbiterator_t *iterator) {
	/*
	 * Move the node cursor to the first node in the database (if any).
	 */

	REQUIRE(DNS_DBITERATOR_VALID(iterator));

	return (iterator->methods->last(iterator));
}

isc_result_t
dns_dbiterator_seek(dns_dbiterator_t *iterator, dns_name_t *name) {
	/*
	 * Move the node cursor to the node with name 'name'.
	 */

	REQUIRE(DNS_DBITERATOR_VALID(iterator));

	return (iterator->methods->seek(iterator, name));
}

isc_result_t
dns_dbiterator_prev(dns_dbiterator_t *iterator) {
	/*
	 * Move the node cursor to the previous node in the database (if any).
	 */

	REQUIRE(DNS_DBITERATOR_VALID(iterator));

	return (iterator->methods->prev(iterator));
}

isc_result_t
dns_dbiterator_next(dns_dbiterator_t *iterator) {
	/*
	 * Move the node cursor to the next node in the database (if any).
	 */

	REQUIRE(DNS_DBITERATOR_VALID(iterator));

	return (iterator->methods->next(iterator));
}

isc_result_t
dns_dbiterator_current(dns_dbiterator_t *iterator, dns_dbnode_t **nodep,
		       dns_name_t *name)
{
	/*
	 * Return the current node.
	 */

	REQUIRE(DNS_DBITERATOR_VALID(iterator));
	REQUIRE(nodep != NULL && *nodep == NULL);
	REQUIRE(name == NULL || dns_name_hasbuffer(name));

	return (iterator->methods->current(iterator, nodep, name));
}

isc_result_t
dns_dbiterator_pause(dns_dbiterator_t *iterator) {
	/*
	 * Pause iteration.
	 */

	REQUIRE(DNS_DBITERATOR_VALID(iterator));

	return (iterator->methods->pause(iterator));
}

isc_result_t
dns_dbiterator_origin(dns_dbiterator_t *iterator, dns_name_t *name) {

	/*
	 * Return the origin to which returned node names are relative.
	 */

	REQUIRE(DNS_DBITERATOR_VALID(iterator));
	REQUIRE(iterator->relative_names);
	REQUIRE(dns_name_hasbuffer(name));

	return (iterator->methods->origin(iterator, name));
}

void
dns_dbiterator_setcleanmode(dns_dbiterator_t *iterator, isc_boolean_t mode) {
	REQUIRE(DNS_DBITERATOR_VALID(iterator));

	iterator->cleaning = mode;
}