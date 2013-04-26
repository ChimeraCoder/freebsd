
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
 * This file implements Solaris compatible zmount() function.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/mount.h>
#include <sys/uio.h>
#include <sys/mntent.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mnttab.h>

static void
build_iovec(struct iovec **iov, int *iovlen, const char *name, void *val,
    size_t len)
{
	int i;

	if (*iovlen < 0)
		return;
	i = *iovlen;
	*iov = realloc(*iov, sizeof(**iov) * (i + 2));
	if (*iov == NULL) {
		*iovlen = -1;
		return;
	}
	(*iov)[i].iov_base = strdup(name);
	(*iov)[i].iov_len = strlen(name) + 1;
	i++;
	(*iov)[i].iov_base = val;
	if (len == (size_t)-1) {
		if (val != NULL)
			len = strlen(val) + 1;
		else
			len = 0;
	}
	(*iov)[i].iov_len = (int)len;
	*iovlen = ++i;
}

int
zmount(const char *spec, const char *dir, int mflag, char *fstype,
    char *dataptr, int datalen, char *optptr, int optlen)
{
	struct iovec *iov;
	char *optstr, *os, *p;
	int iovlen, rv;

	assert(spec != NULL);
	assert(dir != NULL);
	assert(mflag == 0 || mflag == MS_RDONLY);
	assert(fstype != NULL);
	assert(strcmp(fstype, MNTTYPE_ZFS) == 0);
	assert(dataptr == NULL);
	assert(datalen == 0);
	assert(optptr != NULL);
	assert(optlen > 0);

	optstr = strdup(optptr);
	assert(optstr != NULL);

	iov = NULL;
	iovlen = 0;
	if (mflag & MS_RDONLY)
		build_iovec(&iov, &iovlen, "ro", NULL, 0);
	build_iovec(&iov, &iovlen, "fstype", fstype, (size_t)-1);
	build_iovec(&iov, &iovlen, "fspath", __DECONST(char *, dir),
	    (size_t)-1);
	build_iovec(&iov, &iovlen, "from", __DECONST(char *, spec), (size_t)-1);
	for (p = optstr; p != NULL; strsep(&p, ",/ ")) {
		if (*p != '\0')
			build_iovec(&iov, &iovlen, p, NULL, (size_t)-1);
	}
	rv = nmount(iov, iovlen, 0);
	free(optstr);
	return (rv);
}