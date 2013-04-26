
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

/*
 * Principal Author: Brian Wellington
 * $Id: dst_lib.c,v 1.5 2007/06/19 23:47:16 tbox Exp $
 */

/*! \file */

#include <config.h>

#include <stddef.h>

#include <isc/once.h>
#include <isc/msgcat.h>
#include <isc/util.h>

#include <dst/lib.h>

/***
 *** Globals
 ***/

LIBDNS_EXTERNAL_DATA isc_msgcat_t *		dst_msgcat = NULL;


/***
 *** Private
 ***/

static isc_once_t		msgcat_once = ISC_ONCE_INIT;


/***
 *** Functions
 ***/

static void
open_msgcat(void) {
	isc_msgcat_open("libdst.cat", &dst_msgcat);
}

void
dst_lib_initmsgcat(void) {

	/*
	 * Initialize the DST library's message catalog, dst_msgcat, if it
	 * has not already been initialized.
	 */

	RUNTIME_CHECK(isc_once_do(&msgcat_once, open_msgcat) == ISC_R_SUCCESS);
}