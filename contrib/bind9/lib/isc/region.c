
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

/* $Id: region.c,v 1.7 2007/06/19 23:47:17 tbox Exp $ */

/*! \file */

#include <config.h>

#include <stdlib.h>
#include <string.h>

#include <isc/region.h>
#include <isc/util.h>

int
isc_region_compare(isc_region_t *r1, isc_region_t *r2) {
	unsigned int l;
	int result;

	REQUIRE(r1 != NULL);
	REQUIRE(r2 != NULL);
	       
	l = (r1->length < r2->length) ? r1->length : r2->length;

	if ((result = memcmp(r1->base, r2->base, l)) != 0)
		return ((result < 0) ? -1 : 1);
	else
		return ((r1->length == r2->length) ? 0 :
			(r1->length < r2->length) ? -1 : 1);
}