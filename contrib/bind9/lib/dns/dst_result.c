
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

/*%
 * Principal Author: Brian Wellington
 * $Id: dst_result.c,v 1.7 2008/04/01 23:47:10 tbox Exp $
 */

#include <config.h>

#include <isc/once.h>
#include <isc/util.h>

#include <dst/result.h>
#include <dst/lib.h>

static const char *text[DST_R_NRESULTS] = {
	"algorithm is unsupported",		/*%< 0 */
	"crypto failure",			/*%< 1 */
	"built with no crypto support",		/*%< 2 */
	"illegal operation for a null key",	/*%< 3 */
	"public key is invalid",		/*%< 4 */
	"private key is invalid",		/*%< 5 */
	"UNUSED6",				/*%< 6 */
	"error occurred writing key to disk",	/*%< 7 */
	"invalid algorithm specific parameter",	/*%< 8 */
	"UNUSED9",				/*%< 9 */
	"UNUSED10",				/*%< 10 */
	"sign failure",				/*%< 11 */
	"UNUSED12",				/*%< 12 */
	"UNUSED13",				/*%< 13 */
	"verify failure",			/*%< 14 */
	"not a public key",			/*%< 15 */
	"not a private key",			/*%< 16 */
	"not a key that can compute a secret",	/*%< 17 */
	"failure computing a shared secret",	/*%< 18 */
	"no randomness available",		/*%< 19 */
	"bad key type",				/*%< 20 */
	"no engine"				/*%< 21 */
};

#define DST_RESULT_RESULTSET			2

static isc_once_t		once = ISC_ONCE_INIT;

static void
initialize_action(void) {
	isc_result_t result;

	result = isc_result_register(ISC_RESULTCLASS_DST, DST_R_NRESULTS,
				     text, dst_msgcat, DST_RESULT_RESULTSET);
	if (result != ISC_R_SUCCESS)
		UNEXPECTED_ERROR(__FILE__, __LINE__,
				 "isc_result_register() failed: %u", result);
}

static void
initialize(void) {
	dst_lib_initmsgcat();
	RUNTIME_CHECK(isc_once_do(&once, initialize_action) == ISC_R_SUCCESS);
}

const char *
dst_result_totext(isc_result_t result) {
	initialize();

	return (isc_result_totext(result));
}

void
dst_result_register(void) {
	initialize();
}

/*! \file */