
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

/* $Id: bufferlist.c,v 1.17 2007/06/19 23:47:17 tbox Exp $ */

/*! \file */

#include <config.h>

#include <stddef.h>

#include <isc/buffer.h>
#include <isc/bufferlist.h>
#include <isc/util.h>

unsigned int
isc_bufferlist_usedcount(isc_bufferlist_t *bl) {
	isc_buffer_t *buffer;
	unsigned int length;

	REQUIRE(bl != NULL);

	length = 0;
	buffer = ISC_LIST_HEAD(*bl);
	while (buffer != NULL) {
		REQUIRE(ISC_BUFFER_VALID(buffer));
		length += isc_buffer_usedlength(buffer);
		buffer = ISC_LIST_NEXT(buffer, link);
	}

	return (length);
}

unsigned int
isc_bufferlist_availablecount(isc_bufferlist_t *bl) {
	isc_buffer_t *buffer;
	unsigned int length;

	REQUIRE(bl != NULL);

	length = 0;
	buffer = ISC_LIST_HEAD(*bl);
	while (buffer != NULL) {
		REQUIRE(ISC_BUFFER_VALID(buffer));
		length += isc_buffer_availablelength(buffer);
		buffer = ISC_LIST_NEXT(buffer, link);
	}

	return (length);
}