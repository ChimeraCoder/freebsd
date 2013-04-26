
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

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/stdint.h>
#include <sys/ucred.h>
#include <sys/disk.h>
#include <sys/mount.h>

#include <fs/nandfs/nandfs_fs.h>
#include <libnandfs.h>

#define	NANDFS_IS_VALID		0x1
#define	NANDFS_IS_OPENED	0x2
#define	NANDFS_IS_OPENED_DEV	0x4
#define	NANDFS_IS_ERROR		0x8

#define DEBUG
#undef DEBUG
#ifdef DEBUG
#define NANDFS_DEBUG(fmt, args...) do { \
    printf("libnandfs:" fmt "\n", ##args); } while (0)
#else
#define NANDFS_DEBUG(fmt, args...)
#endif

#define	NANDFS_ASSERT_VALID(fs)		assert((fs)->n_flags & NANDFS_IS_VALID)
#define	NANDFS_ASSERT_VALID_DEV(fs)	\
	assert(((fs)->n_flags & (NANDFS_IS_VALID | NANDFS_IS_OPENED_DEV)) == \
	    (NANDFS_IS_VALID | NANDFS_IS_OPENED_DEV))

int
nandfs_iserror(struct nandfs *fs)
{

	NANDFS_ASSERT_VALID(fs);

	return (fs->n_flags & NANDFS_IS_ERROR);
}

const char *
nandfs_errmsg(struct nandfs *fs)
{

	NANDFS_ASSERT_VALID(fs);

	assert(nandfs_iserror(fs));
	assert(fs->n_errmsg);
	return (fs->n_errmsg);
}

static void
nandfs_seterr(struct nandfs *fs, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(fs->n_errmsg, sizeof(fs->n_errmsg), fmt, ap);
	va_end(ap);
	fs->n_flags |= NANDFS_IS_ERROR;
}

const char *
nandfs_dev(struct nandfs *fs)
{

	NANDFS_ASSERT_VALID(fs);
	return (fs->n_dev);
}

void
nandfs_init(struct nandfs *fs, const char *dir)
{

	snprintf(fs->n_ioc, sizeof(fs->n_ioc), "%s/%s", dir, ".");
	fs->n_iocfd = -1;
	fs->n_flags = NANDFS_IS_VALID;
}

void
nandfs_destroy(struct nandfs *fs)
{

	assert(fs->n_iocfd == -1);
	fs->n_flags &=
	    ~(NANDFS_IS_ERROR | NANDFS_IS_VALID);
	assert(fs->n_flags == 0);
}

int
nandfs_open(struct nandfs *fs)
{
	struct nandfs_fsinfo fsinfo;

	fs->n_flags |= NANDFS_IS_OPENED;

	fs->n_iocfd = open(fs->n_ioc, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP |
	    S_IWGRP | S_IROTH | S_IWOTH);
	if (fs->n_iocfd == -1) {
		nandfs_seterr(fs, "couldn't open %s: %s", fs->n_ioc,
		    strerror(errno));
		return (-1);
	}

	if (ioctl(fs->n_iocfd, NANDFS_IOCTL_GET_FSINFO, &fsinfo) == -1) {
		nandfs_seterr(fs, "couldn't fetch fsinfo: %s",
		    strerror(errno));
		return (-1);
	}

	memcpy(&fs->n_fsdata, &fsinfo.fs_fsdata, sizeof(fs->n_fsdata));
	memcpy(&fs->n_sb, &fsinfo.fs_super, sizeof(fs->n_sb));
	snprintf(fs->n_dev, sizeof(fs->n_dev), "%s", fsinfo.fs_dev);

	return (0);
}

void
nandfs_close(struct nandfs *fs)
{

	NANDFS_ASSERT_VALID(fs);
	assert(fs->n_flags & NANDFS_IS_OPENED);

	close(fs->n_iocfd);
	fs->n_iocfd = -1;
	fs->n_flags &= ~NANDFS_IS_OPENED;
}

int
nandfs_get_cpstat(struct nandfs *fs, struct nandfs_cpstat *cpstat)
{

	NANDFS_ASSERT_VALID(fs);

	if (ioctl(fs->n_iocfd, NANDFS_IOCTL_GET_CPSTAT, cpstat) == -1) {
		nandfs_seterr(fs, "ioctl NANDFS_IOCTL_GET_CPSTAT: %s",
		    strerror(errno));
		return (-1);
	}

	return (0);
}

static ssize_t
nandfs_get_cpinfo(struct nandfs *fs, uint64_t cno, int mode,
    struct nandfs_cpinfo *cpinfo, size_t nci)
{
	struct nandfs_argv args;

	NANDFS_ASSERT_VALID(fs);

	args.nv_base = (u_long)cpinfo;
	args.nv_nmembs = nci;
	args.nv_index = cno;
	args.nv_flags = mode;

	if (ioctl(fs->n_iocfd, NANDFS_IOCTL_GET_CPINFO, &args) == -1) {
		nandfs_seterr(fs, "ioctl NANDFS_IOCTL_GET_CPINFO: %s",
		    strerror(errno));
		return (-1);
	}

	return (args.nv_nmembs);
}

ssize_t
nandfs_get_cp(struct nandfs *fs, uint64_t cno, struct nandfs_cpinfo *cpinfo,
    size_t nci)
{

	return (nandfs_get_cpinfo(fs, cno, NANDFS_CHECKPOINT, cpinfo, nci));
}

ssize_t
nandfs_get_snap(struct nandfs *fs, uint64_t cno, struct nandfs_cpinfo *cpinfo,
    size_t nci)
{

	return (nandfs_get_cpinfo(fs, cno, NANDFS_SNAPSHOT, cpinfo, nci));
}

int
nandfs_make_snap(struct nandfs *fs, uint64_t *cno)
{

	NANDFS_ASSERT_VALID(fs);

	if (ioctl(fs->n_iocfd, NANDFS_IOCTL_MAKE_SNAP, cno) == -1) {
		nandfs_seterr(fs, "ioctl NANDFS_IOCTL_MAKE_SNAP: %s",
		    strerror(errno));
		return (-1);
	}

	return (0);
}

int
nandfs_delete_snap(struct nandfs *fs, uint64_t cno)
{

	NANDFS_ASSERT_VALID(fs);

	if (ioctl(fs->n_iocfd, NANDFS_IOCTL_DELETE_SNAP, &cno) == -1) {
		nandfs_seterr(fs, "ioctl NANDFS_IOCTL_DELETE_SNAP: %s",
		    strerror(errno));
		return (-1);
	}

	return (0);
}