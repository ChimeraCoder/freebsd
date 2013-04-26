
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

/* $Id: portset.c,v 1.4 2008/06/24 23:24:35 marka Exp $ */

/*! \file */

#include <config.h>

#include <isc/mem.h>
#include <isc/portset.h>
#include <isc/string.h>
#include <isc/types.h>
#include <isc/util.h>

#define ISC_PORTSET_BUFSIZE (65536 / (sizeof(isc_uint32_t) * 8))

/*%
 * Internal representation of portset.  It's an array of 32-bit integers, each
 * bit corresponding to a single port in the ascending order.  For example,
 * the second most significant bit of buf[0] corresponds to port 1.
 */
struct isc_portset {
	unsigned int nports;	/*%< number of ports in the set */
	isc_uint32_t buf[ISC_PORTSET_BUFSIZE];
};

static inline isc_boolean_t
portset_isset(isc_portset_t *portset, in_port_t port) {
	return (ISC_TF((portset->buf[port >> 5] & (1 << (port & 31))) != 0));
}

static inline void
portset_add(isc_portset_t *portset, in_port_t port) {
	if (!portset_isset(portset, port)) {
		portset->nports++;
		portset->buf[port >> 5] |= (1 << (port & 31));
	}
}

static inline void
portset_remove(isc_portset_t *portset, in_port_t port) {
	if (portset_isset(portset, port)) {
		portset->nports--;
		portset->buf[port >> 5] &= ~(1 << (port & 31));
	}
}

isc_result_t
isc_portset_create(isc_mem_t *mctx, isc_portset_t **portsetp) {
	isc_portset_t *portset;

	REQUIRE(portsetp != NULL && *portsetp == NULL);

	portset = isc_mem_get(mctx, sizeof(*portset));
	if (portset == NULL)
		return (ISC_R_NOMEMORY);

	/* Make the set 'empty' by default */
	memset(portset, 0, sizeof(*portset));
	*portsetp = portset;

	return (ISC_R_SUCCESS);
}

void
isc_portset_destroy(isc_mem_t *mctx, isc_portset_t **portsetp) {
	isc_portset_t *portset;

	REQUIRE(portsetp != NULL);
	portset = *portsetp;

	isc_mem_put(mctx, portset, sizeof(*portset));
}

isc_boolean_t
isc_portset_isset(isc_portset_t *portset, in_port_t port) {
	REQUIRE(portset != NULL);

	return (portset_isset(portset, port));
}

unsigned int
isc_portset_nports(isc_portset_t *portset) {
	REQUIRE(portset != NULL);

	return (portset->nports);
}

void
isc_portset_add(isc_portset_t *portset, in_port_t port) {
	REQUIRE(portset != NULL);

	portset_add(portset, port);
}

void
isc_portset_remove(isc_portset_t *portset, in_port_t port) {
	portset_remove(portset, port);
}

void
isc_portset_addrange(isc_portset_t *portset, in_port_t port_lo,
		     in_port_t port_hi)
{
	in_port_t p;

	REQUIRE(portset != NULL);
	REQUIRE(port_lo <= port_hi);

	p = port_lo;
	do {
		portset_add(portset, p);
	} while (p++ < port_hi);
}

void
isc_portset_removerange(isc_portset_t *portset, in_port_t port_lo,
			in_port_t port_hi)
{
	in_port_t p;

	REQUIRE(portset != NULL);
	REQUIRE(port_lo <= port_hi);

	p = port_lo;
	do {
		portset_remove(portset, p);
	} while (p++ < port_hi);
}