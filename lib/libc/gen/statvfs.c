
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

#include "namespace.h"
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/statvfs.h>

#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include "un-namespace.h"

static int	sfs2svfs(const struct statfs *from, struct statvfs *to);

int
fstatvfs(int fd, struct statvfs *result)
{
	struct statfs sfs;
	int rv;
	long pcval;

	rv = _fstatfs(fd, &sfs);
	if (rv != 0)
		return (rv);

	rv = sfs2svfs(&sfs, result);
	if (rv != 0)
		return (rv);

	/*
	 * Whether pathconf's -1 return means error or unlimited does not
	 * make any difference in this best-effort implementation.
	 */
	pcval = _fpathconf(fd, _PC_NAME_MAX);
	if (pcval == -1)
		result->f_namemax = ~0UL;
	else
		result->f_namemax = (unsigned long)pcval;
	return (0);
}

int
statvfs(const char * __restrict path, struct statvfs * __restrict result)
{
	struct statfs sfs;
	int rv;
	long pcval;

	rv = statfs(path, &sfs);
	if (rv != 0)
		return (rv);

	sfs2svfs(&sfs, result);

	/*
	 * Whether pathconf's -1 return means error or unlimited does not
	 * make any difference in this best-effort implementation.
	 */
	pcval = pathconf(path, _PC_NAME_MAX);
	if (pcval == -1)
		result->f_namemax = ~0UL;
	else
		result->f_namemax = (unsigned long)pcval;
	return (0);
}

static int
sfs2svfs(const struct statfs *from, struct statvfs *to)
{
	static const struct statvfs zvfs;

	*to = zvfs;

	if (from->f_flags & MNT_RDONLY)
		to->f_flag |= ST_RDONLY;
	if (from->f_flags & MNT_NOSUID)
		to->f_flag |= ST_NOSUID;

	/* XXX should we clamp negative values? */
#define COPY(field) \
	do { \
		to->field = from->field; \
		if (from->field != to->field) { \
			errno = EOVERFLOW; \
			return (-1); \
		} \
	} while(0)

	COPY(f_bavail);
	COPY(f_bfree);
	COPY(f_blocks);
	COPY(f_ffree);
	COPY(f_files);
	to->f_bsize = from->f_iosize;
	to->f_frsize = from->f_bsize;
	to->f_favail = to->f_ffree;
	return (0);
}
								 
#ifdef MAIN
#include <err.h>
#include <stdint.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
	struct statvfs buf;

	if (statvfs(argv[1], &buf) < 0)
		err(1, "statvfs");

#define SHOW(field) \
	printf(#field ": %ju\n", (uintmax_t)buf.field)

	SHOW(f_bavail);
	SHOW(f_bfree);
	SHOW(f_blocks);
	SHOW(f_favail);
	SHOW(f_ffree);
	SHOW(f_files);
	SHOW(f_bsize);
	SHOW(f_frsize);
	SHOW(f_namemax);
	printf("f_flag: %lx\n", (unsigned long)buf.f_flag);

	return 0;
}

#endif /* MAIN */