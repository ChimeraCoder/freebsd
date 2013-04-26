
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
static char sccsid[] = "@(#)devname.c	8.2 (Berkeley) 4/29/95";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/sysctl.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>

char *
devname_r(dev_t dev, mode_t type, char *buf, int len)
{
	int i;
	size_t j;

	if (dev == NODEV || !(S_ISCHR(type) || S_ISBLK(dev))) {
		strlcpy(buf, "#NODEV", len);
		return (buf);
	}

	if (S_ISCHR(type)) {
		j = len;
		i = sysctlbyname("kern.devname", buf, &j, &dev, sizeof (dev));
		if (i == 0)
			return (buf);
	}

	/* Finally just format it */
	snprintf(buf, len, "#%c:%#jx",
	    S_ISCHR(type) ? 'C' : 'B', (uintmax_t)dev);
	return (buf);
}

char *
devname(dev_t dev, mode_t type)
{
	static char buf[SPECNAMELEN + 1];

	return (devname_r(dev, type, buf, sizeof(buf)));
}