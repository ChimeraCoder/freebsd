
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/sysctl.h>

#include <errno.h>
#include <paths.h>
#include <stdio.h>
#include <string.h>

#define	LEN_PATH_DEV	(sizeof(_PATH_DEV) - 1)

char *
ctermid(char *s)
{
	static char def[sizeof(_PATH_DEV) + SPECNAMELEN];
	struct stat sb;
	size_t dlen;
	int sverrno;

	if (s == NULL) {
		s = def;
		dlen = sizeof(def) - LEN_PATH_DEV;
	} else
		dlen = L_ctermid - LEN_PATH_DEV;
	strcpy(s, _PATH_TTY);

	/* Attempt to perform a lookup of the actual TTY pathname. */
	sverrno = errno;
	if (stat(_PATH_TTY, &sb) == 0 && S_ISCHR(sb.st_mode))
		(void)sysctlbyname("kern.devname", s + LEN_PATH_DEV,
		    &dlen, &sb.st_rdev, sizeof(sb.st_rdev));
	errno = sverrno;
	return (s);
}

char *
ctermid_r(char *s)
{

	return (s != NULL ? ctermid(s) : NULL);
}