
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

/* $Id: fsaccess.c,v 1.13 2007/06/19 23:47:18 tbox Exp $ */

#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>

#include "errno2result.h"

/*! \file
 * \brief
 * The OS-independent part of the API is in lib/isc.
 */
#include "../fsaccess.c"

isc_result_t
isc_fsaccess_set(const char *path, isc_fsaccess_t access) {
	struct stat statb;
	mode_t mode;
	isc_boolean_t is_dir = ISC_FALSE;
	isc_fsaccess_t bits;
	isc_result_t result;

	if (stat(path, &statb) != 0)
		return (isc__errno2result(errno));

	if ((statb.st_mode & S_IFDIR) != 0)
		is_dir = ISC_TRUE;
	else if ((statb.st_mode & S_IFREG) == 0)
		return (ISC_R_INVALIDFILE);

	result = check_bad_bits(access, is_dir);
	if (result != ISC_R_SUCCESS)
		return (result);

	/*
	 * Done with checking bad bits.  Set mode_t.
	 */
	mode = 0;

#define SET_AND_CLEAR1(modebit) \
	if ((access & bits) != 0) { \
		mode |= modebit; \
		access &= ~bits; \
	}
#define SET_AND_CLEAR(user, group, other) \
	SET_AND_CLEAR1(user); \
	bits <<= STEP; \
	SET_AND_CLEAR1(group); \
	bits <<= STEP; \
	SET_AND_CLEAR1(other);

	bits = ISC_FSACCESS_READ | ISC_FSACCESS_LISTDIRECTORY;

	SET_AND_CLEAR(S_IRUSR, S_IRGRP, S_IROTH);

	bits = ISC_FSACCESS_WRITE |
	       ISC_FSACCESS_CREATECHILD |
	       ISC_FSACCESS_DELETECHILD;

	SET_AND_CLEAR(S_IWUSR, S_IWGRP, S_IWOTH);

	bits = ISC_FSACCESS_EXECUTE |
	       ISC_FSACCESS_ACCESSCHILD;

	SET_AND_CLEAR(S_IXUSR, S_IXGRP, S_IXOTH);

	INSIST(access == 0);

	if (chmod(path, mode) < 0)
		return (isc__errno2result(errno));

	return (ISC_R_SUCCESS);
}