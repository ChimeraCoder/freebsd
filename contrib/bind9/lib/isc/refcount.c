
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

/* $Id: refcount.c,v 1.5 2007/06/19 23:47:17 tbox Exp $ */

#include <config.h>

#include <stddef.h>

#include <isc/mutex.h>
#include <isc/refcount.h>
#include <isc/result.h>

isc_result_t
isc_refcount_init(isc_refcount_t *ref, unsigned int n) {
	REQUIRE(ref != NULL);

	ref->refs = n;
#if defined(ISC_PLATFORM_USETHREADS) && !defined(ISC_PLATFORM_HAVEXADD)
	return (isc_mutex_init(&ref->lock));
#else
	return (ISC_R_SUCCESS);
#endif
}