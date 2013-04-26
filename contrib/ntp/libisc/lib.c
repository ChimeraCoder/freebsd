
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

/* $Id: lib.c,v 1.9 2001/11/19 03:08:23 mayer Exp $ */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <isc/once.h>
#include <isc/msgs.h>
#include <isc/lib.h>

/***
 *** Globals
 ***/

LIBISC_EXTERNAL_DATA isc_msgcat_t *		isc_msgcat = NULL;


/***
 *** Private
 ***/

static isc_once_t		msgcat_once = ISC_ONCE_INIT;


/***
 *** Functions
 ***/

static void
open_msgcat(void) {
	isc_msgcat_open("libisc.cat", &isc_msgcat);
}

void
isc_lib_initmsgcat(void) {
	isc_result_t result;

	/*
	 * Initialize the ISC library's message catalog, isc_msgcat, if it
	 * has not already been initialized.
	 */

	result = isc_once_do(&msgcat_once, open_msgcat);
	if (result != ISC_R_SUCCESS) {
		/*
		 * Normally we'd use RUNTIME_CHECK() or FATAL_ERROR(), but
		 * we can't do that here, since they might call us!
		 * (Note that the catalog might be open anyway, so we might
		 * as well try to  provide an internationalized message.)
		 */
		fprintf(stderr, "%s:%d: %s: isc_once_do() %s.\n",
			__FILE__, __LINE__,
			isc_msgcat_get(isc_msgcat, ISC_MSGSET_GENERAL,
				       ISC_MSG_FATALERROR, "fatal error"),
			isc_msgcat_get(isc_msgcat, ISC_MSGSET_GENERAL,
				       ISC_MSG_FAILED, "failed"));
		abort();
	}
}