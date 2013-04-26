
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

/* $Id: assertions.c,v 1.16 2001/07/16 03:52:05 mayer Exp $ */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <isc/assertions.h>
#include <isc/msgs.h>

/*
 * Forward.
 */

static void
default_callback(const char *, int, isc_assertiontype_t, const char *);

/*
 * Public.
 */

LIBISC_EXTERNAL_DATA isc_assertioncallback_t isc_assertion_failed =
					     default_callback;

void
isc_assertion_setcallback(isc_assertioncallback_t cb) {
	if (cb == NULL)
		isc_assertion_failed = default_callback;
	else
		isc_assertion_failed = cb;
}

const char *
isc_assertion_typetotext(isc_assertiontype_t type) {
	const char *result;

	/*
	 * These strings have purposefully not been internationalized
	 * because they are considered to essentially be keywords of
	 * the ISC development environment.
	 */
	switch (type) {
	case isc_assertiontype_require:
		result = "REQUIRE";
		break;
	case isc_assertiontype_ensure:
		result = "ENSURE";
		break;
	case isc_assertiontype_insist:
		result = "INSIST";
		break;
	case isc_assertiontype_invariant:
		result = "INVARIANT";
		break;
	default:
		result = NULL;
	}
	return (result);
}

/*
 * Private.
 */

static void
default_callback(const char *file, int line, isc_assertiontype_t type,
		 const char *cond)
{
	fprintf(stderr, "%s:%d: %s(%s) %s.\n",
		file, line, isc_assertion_typetotext(type), cond,
		isc_msgcat_get(isc_msgcat, ISC_MSGSET_GENERAL,
			       ISC_MSG_FAILED, "failed"));
	fflush(stderr);
	abort();
	/* NOTREACHED */
}