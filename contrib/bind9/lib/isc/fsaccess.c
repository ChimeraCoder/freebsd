
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

/* $Id: fsaccess.c,v 1.10 2007/06/19 23:47:17 tbox Exp $ */

/*! \file
 * \brief
 * This file contains the OS-independent functionality of the API.
 */
#include <isc/fsaccess.h>
#include <isc/result.h>
#include <isc/util.h>

/*!
 * Shorthand.  Maybe ISC__FSACCESS_PERMISSIONBITS should not even be in
 * <isc/fsaccess.h>.  Could check consistency with sizeof(isc_fsaccess_t)
 * and the number of bits in each function.
 */
#define STEP		(ISC__FSACCESS_PERMISSIONBITS)
#define GROUP		(STEP)
#define OTHER		(STEP * 2)

void
isc_fsaccess_add(int trustee, int permission, isc_fsaccess_t *access) {
	REQUIRE(trustee <= 0x7);
	REQUIRE(permission <= 0xFF);

	if ((trustee & ISC_FSACCESS_OWNER) != 0)
		*access |= permission;

	if ((trustee & ISC_FSACCESS_GROUP) != 0)
		*access |= (permission << GROUP);

	if ((trustee & ISC_FSACCESS_OTHER) != 0)
		*access |= (permission << OTHER);
}

void
isc_fsaccess_remove(int trustee, int permission, isc_fsaccess_t *access) {
	REQUIRE(trustee <= 0x7);
	REQUIRE(permission <= 0xFF);


	if ((trustee & ISC_FSACCESS_OWNER) != 0)
		*access &= ~permission;

	if ((trustee & ISC_FSACCESS_GROUP) != 0)
		*access &= ~(permission << GROUP);

	if ((trustee & ISC_FSACCESS_OTHER) != 0)
		*access &= ~(permission << OTHER);
}

static isc_result_t
check_bad_bits(isc_fsaccess_t access, isc_boolean_t is_dir) {
	isc_fsaccess_t bits;

	/*
	 * Check for disallowed user bits.
	 */
	if (is_dir)
		bits = ISC_FSACCESS_READ |
		       ISC_FSACCESS_WRITE |
		       ISC_FSACCESS_EXECUTE;
	else
		bits = ISC_FSACCESS_CREATECHILD |
		       ISC_FSACCESS_ACCESSCHILD |
		       ISC_FSACCESS_DELETECHILD |
		       ISC_FSACCESS_LISTDIRECTORY;

	/*
	 * Set group bad bits.
	 */
	bits |= bits << STEP;
	/*
	 * Set other bad bits.
	 */
	bits |= bits << STEP;

	if ((access & bits) != 0) {
		if (is_dir)
			return (ISC_R_NOTFILE);
		else
			return (ISC_R_NOTDIRECTORY);
	}

	return (ISC_R_SUCCESS);
}