
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

#define _GNU_SOURCE

#if HAVE_CONFIG_H
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <endian.h>
#include <byteswap.h>
#include <sys/poll.h>
#include <syslog.h>
#include <netinet/in.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/sysctl.h>

#include "common.h"

static int
ret_code(void)
{
	int e = errno;

	if (e > 0)
		return -e;
	return e;
}

int
sys_read_string(char *dir_name, char *file_name, char *str, int max_len)
{
	char path[256], *s;
	size_t len;

	snprintf(path, sizeof(path), "%s/%s", dir_name, file_name);

	for (s = &path[0]; *s != '\0'; s++)
		if (*s == '/')
			*s = '.';

	len = max_len;
	if (sysctlbyname(&path[1], str, &len, NULL, 0) == -1)
		return ret_code();

	str[(len < max_len) ? len : max_len - 1] = 0;

	if ((s = strrchr(str, '\n')))
		*s = 0;

	return 0;
}

int
sys_read_guid(char *dir_name, char *file_name, uint64_t *net_guid)
{
	char buf[32], *str, *s;
	uint64_t guid;
	int r, i;

	if ((r = sys_read_string(dir_name, file_name, buf, sizeof(buf))) < 0)
		return r;

	guid = 0;

	for (s = buf, i = 0 ; i < 4; i++) {
		if (!(str = strsep(&s, ": \t\n")))
			return -EINVAL;
		guid = (guid << 16) | (strtoul(str, 0, 16) & 0xffff);
	}

	*net_guid = htonll(guid);

	return 0;
}

int
sys_read_gid(char *dir_name, char *file_name, uint8_t *gid)
{
	char buf[64], *str, *s;
	uint16_t *ugid = (uint16_t *)gid;
	int r, i;

	if ((r = sys_read_string(dir_name, file_name, buf, sizeof(buf))) < 0)
		return r;

	for (s = buf, i = 0 ; i < 8; i++) {
		if (!(str = strsep(&s, ": \t\n"))) 
			return -EINVAL;
		ugid[i] = htons(strtoul(str, 0, 16) & 0xffff);
	}

	return 0;
}

int
sys_read_uint64(char *dir_name, char *file_name, uint64_t *u)
{
	char buf[32];
	int r;

	if ((r = sys_read_string(dir_name, file_name, buf, sizeof(buf))) < 0)
		return r;

	*u = strtoull(buf, 0, 0);

	return 0;
}

int
sys_read_uint(char *dir_name, char *file_name, unsigned *u)
{
	char buf[32];
	int r;

	if ((r = sys_read_string(dir_name, file_name, buf, sizeof(buf))) < 0)
		return r;

	*u = strtoul(buf, 0, 0);

	return 0;
}

#define	DIRECTSIZ(namlen)						\
	(((uintptr_t)&((struct dirent *)0)->d_name +			\
	((namlen)+1)*sizeof(((struct dirent *)0)->d_name[0]) + 3) & ~3)

int
sys_scandir(const char *dirname, struct dirent ***namelist,
    int (*select)(const struct dirent *),
    int (*compar)(const struct dirent **, const struct dirent **))
{
	struct dirent **names;
	struct dirent **names2;
	struct dirent *dp;
	char name[1024];
	int lsname[22];
	int chname[22];
	int name2[22];
	int oid[22];
	char *s;
	size_t n1, n2;
	size_t len, oidlen, namlen;
	int cnt, max;
	int err;
	int i;

	*namelist = NULL;
	/* Skip the leading / */
	strncpy(name, &dirname[1], sizeof(name));
	for (s = &name[0]; *s != '\0'; s++)
		if (*s == '/')
			*s = '.';
	/*
	 * Resolve the path.
	 */
	len = sizeof(oid) / sizeof(int);
	namlen = strlen(name) + 1;
	if (sysctlnametomib(name, oid, &len) != 0)
		return (-errno);
	lsname[0] = 0;	/* Root */
	lsname[1] = 2;	/* Get next */
	memcpy(lsname+2, oid, len * sizeof(int));
	n1 = 2 + len;
	oidlen = len;
	/*
	 * Setup the return list of dirents.
	 */
	cnt = 0;
	max = 64;
	names = malloc(max * sizeof(void *));
	if (names == NULL)
		return (-ENOMEM);

	for (;;) {
		n2 = sizeof(name2);
		if (sysctl(lsname, n1, name2, &n2, 0, 0) < 0) {
			if (errno == ENOENT)
				break;
			goto errout;
		}
		n2 /= sizeof(int);
		if (n2 < oidlen)
			break;
		for (i = 0; i < oidlen; i++)
			if (name2[i] != oid[i])
				goto out;
		chname[0] = 0;	/* root */
		chname[1] = 1;	/* oid name */
		memcpy(chname + 2, name2, n2 * sizeof(int));
		memcpy(lsname + 2, name2, n2 * sizeof(int));
		n1 = 2 + n2;
		/*
		 * scandir() is not supposed to go deeper than the requested
		 * directory but sysctl also doesn't return a node for
		 * 'subdirectories' so we have to find a file in the subdir
		 * and then truncate the name to report it.
	 	 */
		if (n2 > oidlen + 1) {
			/* Skip to the next name after this one. */
			n1 = 2 + oidlen + 1;
			lsname[n1 - 1]++;
		}
		len = sizeof(name);
		if (sysctl(chname, n2 + 2, name, &len, 0, 0) < 0)
			goto errout;
		if (len <= 0 || len < namlen)
			goto out;
		s = name + namlen;
		/* Just keep the first level name. */
		if (strchr(s, '.'))
			*strchr(s, '.') = '\0';
		len = strlen(s) + 1;
		dp = malloc(DIRECTSIZ(len));
		dp->d_reclen = DIRECTSIZ(len);
		dp->d_namlen = len;
		memcpy(&dp->d_name, s, len);
		if (select && !select(dp)) {
			free(dp);
			continue;
		}
		if (cnt == max) {
			max *= 2;
			names2 = realloc(names, max * sizeof(void *));
			if (names2 == NULL) {
				errno = ENOMEM;
				free(dp);
				goto errout;
			}
			names = names2;
		}
		names[cnt++] = dp;
	}
out:
	if (cnt && compar)
		qsort(names, cnt, sizeof(struct dirent *),
		    (int (*)(const void *, const void *))compar);
		
	*namelist = names;

	return (cnt);

errout:
	err = errno;
	for (i = 0; i < cnt; i++)
		free(names[i]);
	free(names);
	return (-err);
}