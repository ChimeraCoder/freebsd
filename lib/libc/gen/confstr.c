
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)confstr.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>

#include <errno.h>
#include <limits.h>
#include <paths.h>
#include <string.h>
#include <unistd.h>


size_t
confstr(int name, char *buf, size_t len)
{
	const char *p;
	const char UPE[] = "unsupported programming environment";

	switch (name) {
	case _CS_PATH:
		p = _PATH_STDPATH;
		goto docopy;

		/*
		 * POSIX/SUS ``Programming Environments'' stuff
		 *
		 * We don't support more than one programming environment
		 * on any platform (yet), so we just return the empty
		 * string for the environment we are compiled for,
		 * and the string "unsupported programming environment"
		 * for anything else.  (The Standard says that if these
		 * values are used on a system which does not support
		 * this environment -- determined via sysconf() -- then
		 * the value we return is unspecified.  So, we return
		 * something which will cause obvious breakage.)
		 */
	case _CS_POSIX_V6_ILP32_OFF32_CFLAGS:
	case _CS_POSIX_V6_ILP32_OFF32_LDFLAGS:
	case _CS_POSIX_V6_ILP32_OFF32_LIBS:
	case _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS:
	case _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS:
	case _CS_POSIX_V6_LPBIG_OFFBIG_LIBS:
		/*
		 * These two environments are never supported.
		 */
		p = UPE;
		goto docopy;

	case _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS:
	case _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS:
	case _CS_POSIX_V6_ILP32_OFFBIG_LIBS:
		if (sizeof(long) * CHAR_BIT == 32 &&
		    sizeof(off_t) > sizeof(long))
			p = "";
		else
			p = UPE;
		goto docopy;

	case _CS_POSIX_V6_LP64_OFF64_CFLAGS:
	case _CS_POSIX_V6_LP64_OFF64_LDFLAGS:
	case _CS_POSIX_V6_LP64_OFF64_LIBS:
		if (sizeof(long) * CHAR_BIT >= 64 &&
		    sizeof(void *) * CHAR_BIT >= 64 &&
		    sizeof(int) * CHAR_BIT >= 32 &&
		    sizeof(off_t) >= sizeof(long))
			p = "";
		else
			p = UPE;
		goto docopy;

	case _CS_POSIX_V6_WIDTH_RESTRICTED_ENVS:
		/* XXX - should have more complete coverage */
		if (sizeof(long) * CHAR_BIT >= 64)
			p = "_POSIX_V6_LP64_OFF64";
		else
			p = "_POSIX_V6_ILP32_OFFBIG";
		goto docopy;

docopy:
		if (len != 0 && buf != NULL)
			strlcpy(buf, p, len);
		return (strlen(p) + 1);

	default:
		errno = EINVAL;
		return (0);
	}
	/* NOTREACHED */
}