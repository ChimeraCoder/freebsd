
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
 * Common name validation routines for ZFS.  These routines are shared by the
 * userland code as well as the ioctl() layer to ensure that we don't
 * inadvertently expose a hole through direct ioctl()s that never gets tested.
 * In userland, however, we want significantly more information about _why_ the
 * name is invalid.  In the kernel, we only care whether it's valid or not.
 * Each routine therefore takes a 'namecheck_err_t' which describes exactly why
 * the name failed to validate.
 *
 * Each function returns 0 on success, -1 on error.
 */

#if defined(_KERNEL)
#include <sys/systm.h>
#else
#include <string.h>
#endif

#include <sys/param.h>
#include <sys/nvpair.h>
#include "zfs_namecheck.h"
#include "zfs_deleg.h"

static int
valid_char(char c)
{
	return ((c >= 'a' && c <= 'z') ||
	    (c >= 'A' && c <= 'Z') ||
	    (c >= '0' && c <= '9') ||
	    c == '-' || c == '_' || c == '.' || c == ':' || c == ' ');
}

/*
 * Snapshot names must be made up of alphanumeric characters plus the following
 * characters:
 *
 * 	[-_.: ]
 */
int
snapshot_namecheck(const char *path, namecheck_err_t *why, char *what)
{
	const char *loc;

	if (strlen(path) >= MAXNAMELEN) {
		if (why)
			*why = NAME_ERR_TOOLONG;
		return (-1);
	}

	if (path[0] == '\0') {
		if (why)
			*why = NAME_ERR_EMPTY_COMPONENT;
		return (-1);
	}

	for (loc = path; *loc; loc++) {
		if (!valid_char(*loc)) {
			if (why) {
				*why = NAME_ERR_INVALCHAR;
				*what = *loc;
			}
			return (-1);
		}
	}
	return (0);
}


/*
 * Permissions set name must start with the letter '@' followed by the
 * same character restrictions as snapshot names, except that the name
 * cannot exceed 64 characters.
 */
int
permset_namecheck(const char *path, namecheck_err_t *why, char *what)
{
	if (strlen(path) >= ZFS_PERMSET_MAXLEN) {
		if (why)
			*why = NAME_ERR_TOOLONG;
		return (-1);
	}

	if (path[0] != '@') {
		if (why) {
			*why = NAME_ERR_NO_AT;
			*what = path[0];
		}
		return (-1);
	}

	return (snapshot_namecheck(&path[1], why, what));
}

/*
 * Dataset names must be of the following form:
 *
 * 	[component][/]*[component][@component]
 *
 * Where each component is made up of alphanumeric characters plus the following
 * characters:
 *
 * 	[-_.:%]
 *
 * We allow '%' here as we use that character internally to create unique
 * names for temporary clones (for online recv).
 */
int
dataset_namecheck(const char *path, namecheck_err_t *why, char *what)
{
	const char *loc, *end;
	int found_snapshot;

	/*
	 * Make sure the name is not too long.
	 *
	 * ZFS_MAXNAMELEN is the maximum dataset length used in the userland
	 * which is the same as MAXNAMELEN used in the kernel.
	 * If ZFS_MAXNAMELEN value is changed, make sure to cleanup all
	 * places using MAXNAMELEN.
	 */

	if (strlen(path) >= MAXNAMELEN) {
		if (why)
			*why = NAME_ERR_TOOLONG;
		return (-1);
	}

	/* Explicitly check for a leading slash.  */
	if (path[0] == '/') {
		if (why)
			*why = NAME_ERR_LEADING_SLASH;
		return (-1);
	}

	if (path[0] == '\0') {
		if (why)
			*why = NAME_ERR_EMPTY_COMPONENT;
		return (-1);
	}

	loc = path;
	found_snapshot = 0;
	for (;;) {
		/* Find the end of this component */
		end = loc;
		while (*end != '/' && *end != '@' && *end != '\0')
			end++;

		if (*end == '\0' && end[-1] == '/') {
			/* trailing slashes are not allowed */
			if (why)
				*why = NAME_ERR_TRAILING_SLASH;
			return (-1);
		}

		/* Zero-length components are not allowed */
		if (loc == end) {
			if (why) {
				/*
				 * Make sure this is really a zero-length
				 * component and not a '@@'.
				 */
				if (*end == '@' && found_snapshot) {
					*why = NAME_ERR_MULTIPLE_AT;
				} else {
					*why = NAME_ERR_EMPTY_COMPONENT;
				}
			}

			return (-1);
		}

		/* Validate the contents of this component */
		while (loc != end) {
			if (!valid_char(*loc) && *loc != '%') {
				if (why) {
					*why = NAME_ERR_INVALCHAR;
					*what = *loc;
				}
				return (-1);
			}
			loc++;
		}

		/* If we've reached the end of the string, we're OK */
		if (*end == '\0')
			return (0);

		if (*end == '@') {
			/*
			 * If we've found an @ symbol, indicate that we're in
			 * the snapshot component, and report a second '@'
			 * character as an error.
			 */
			if (found_snapshot) {
				if (why)
					*why = NAME_ERR_MULTIPLE_AT;
				return (-1);
			}

			found_snapshot = 1;
		}

		/*
		 * If there is a '/' in a snapshot name
		 * then report an error
		 */
		if (*end == '/' && found_snapshot) {
			if (why)
				*why = NAME_ERR_TRAILING_SLASH;
			return (-1);
		}

		/* Update to the next component */
		loc = end + 1;
	}
}


/*
 * mountpoint names must be of the following form:
 *
 *	/[component][/]*[component][/]
 */
int
mountpoint_namecheck(const char *path, namecheck_err_t *why)
{
	const char *start, *end;

	/*
	 * Make sure none of the mountpoint component names are too long.
	 * If a component name is too long then the mkdir of the mountpoint
	 * will fail but then the mountpoint property will be set to a value
	 * that can never be mounted.  Better to fail before setting the prop.
	 * Extra slashes are OK, they will be tossed by the mountpoint mkdir.
	 */

	if (path == NULL || *path != '/') {
		if (why)
			*why = NAME_ERR_LEADING_SLASH;
		return (-1);
	}

	/* Skip leading slash  */
	start = &path[1];
	do {
		end = start;
		while (*end != '/' && *end != '\0')
			end++;

		if (end - start >= MAXNAMELEN) {
			if (why)
				*why = NAME_ERR_TOOLONG;
			return (-1);
		}
		start = end + 1;

	} while (*end != '\0');

	return (0);
}

/*
 * For pool names, we have the same set of valid characters as described in
 * dataset names, with the additional restriction that the pool name must begin
 * with a letter.  The pool names 'raidz' and 'mirror' are also reserved names
 * that cannot be used.
 */
int
pool_namecheck(const char *pool, namecheck_err_t *why, char *what)
{
	const char *c;

	/*
	 * Make sure the name is not too long.
	 *
	 * ZPOOL_MAXNAMELEN is the maximum pool length used in the userland
	 * which is the same as MAXNAMELEN used in the kernel.
	 * If ZPOOL_MAXNAMELEN value is changed, make sure to cleanup all
	 * places using MAXNAMELEN.
	 */
	if (strlen(pool) >= MAXNAMELEN) {
		if (why)
			*why = NAME_ERR_TOOLONG;
		return (-1);
	}

	c = pool;
	while (*c != '\0') {
		if (!valid_char(*c)) {
			if (why) {
				*why = NAME_ERR_INVALCHAR;
				*what = *c;
			}
			return (-1);
		}
		c++;
	}

	if (!(*pool >= 'a' && *pool <= 'z') &&
	    !(*pool >= 'A' && *pool <= 'Z')) {
		if (why)
			*why = NAME_ERR_NOLETTER;
		return (-1);
	}

	if (strcmp(pool, "mirror") == 0 || strcmp(pool, "raidz") == 0) {
		if (why)
			*why = NAME_ERR_RESERVED;
		return (-1);
	}

	if (pool[0] == 'c' && (pool[1] >= '0' && pool[1] <= '9')) {
		if (why)
			*why = NAME_ERR_DISKLIKE;
		return (-1);
	}

	return (0);
}