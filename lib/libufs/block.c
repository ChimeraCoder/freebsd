
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
#include <sys/mount.h>
#include <sys/disk.h>
#include <sys/disklabel.h>
#include <sys/stat.h>

#include <ufs/ufs/ufsmount.h>
#include <ufs/ufs/dinode.h>
#include <ufs/ffs/fs.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libufs.h>

ssize_t
bread(struct uufsd *disk, ufs2_daddr_t blockno, void *data, size_t size)
{
	void *p2;
	ssize_t cnt;

	ERROR(disk, NULL);

	p2 = data;
	/*
	 * XXX: various disk controllers require alignment of our buffer
	 * XXX: which is stricter than struct alignment.
	 * XXX: Bounce the buffer if not 64 byte aligned.
	 * XXX: this can be removed if/when the kernel is fixed
	 */
	if (((intptr_t)data) & 0x3f) {
		p2 = malloc(size);
		if (p2 == NULL) {
			ERROR(disk, "allocate bounce buffer");
			goto fail;
		}
	}
	cnt = pread(disk->d_fd, p2, size, (off_t)(blockno * disk->d_bsize));
	if (cnt == -1) {
		ERROR(disk, "read error from block device");
		goto fail;
	}
	if (cnt == 0) {
		ERROR(disk, "end of file from block device");
		goto fail;
	}
	if ((size_t)cnt != size) {
		ERROR(disk, "short read or read error from block device");
		goto fail;
	}
	if (p2 != data) {
		memcpy(data, p2, size);
		free(p2);
	}
	return (cnt);
fail:	memset(data, 0, size);
	if (p2 != data) {
		free(p2);
	}
	return (-1);
}

ssize_t
bwrite(struct uufsd *disk, ufs2_daddr_t blockno, const void *data, size_t size)
{
	ssize_t cnt;
	int rv;
	void *p2 = NULL;

	ERROR(disk, NULL);

	rv = ufs_disk_write(disk);
	if (rv == -1) {
		ERROR(disk, "failed to open disk for writing");
		return (-1);
	}

	/*
	 * XXX: various disk controllers require alignment of our buffer
	 * XXX: which is stricter than struct alignment.
	 * XXX: Bounce the buffer if not 64 byte aligned.
	 * XXX: this can be removed if/when the kernel is fixed
	 */
	if (((intptr_t)data) & 0x3f) {
		p2 = malloc(size);
		if (p2 == NULL) {
			ERROR(disk, "allocate bounce buffer");
			return (-1);
		}
		memcpy(p2, data, size);
		data = p2;
	}
	cnt = pwrite(disk->d_fd, data, size, (off_t)(blockno * disk->d_bsize));
	if (p2 != NULL)
		free(p2);
	if (cnt == -1) {
		ERROR(disk, "write error to block device");
		return (-1);
	}
	if ((size_t)cnt != size) {
		ERROR(disk, "short write to block device");
		return (-1);
	}

	return (cnt);
}

#ifdef __FreeBSD_kernel__

static int
berase_helper(struct uufsd *disk, ufs2_daddr_t blockno, ufs2_daddr_t size)
{
	off_t ioarg[2];

	ioarg[0] = blockno * disk->d_bsize;
	ioarg[1] = size;
	return (ioctl(disk->d_fd, DIOCGDELETE, ioarg));
}

#else

static int
berase_helper(struct uufsd *disk, ufs2_daddr_t blockno, ufs2_daddr_t size)
{
	char *zero_chunk;
	off_t offset, zero_chunk_size, pwrite_size;
	int rv;

	offset = blockno * disk->d_bsize;
	zero_chunk_size = 65536 * disk->d_bsize;
	zero_chunk = calloc(1, zero_chunk_size);
	if (zero_chunk == NULL) {
		ERROR(disk, "failed to allocate memory");
		return (-1);
	}
	while (size > 0) { 
		pwrite_size = size;
		if (pwrite_size > zero_chunk_size)
			pwrite_size = zero_chunk_size;
		rv = pwrite(disk->d_fd, zero_chunk, pwrite_size, offset);
		if (rv == -1) {
			ERROR(disk, "failed writing to disk");
			break;
		}
		size -= rv;
		offset += rv;
		rv = 0;
	}
	free(zero_chunk);
	return (rv);
}

#endif

int
berase(struct uufsd *disk, ufs2_daddr_t blockno, ufs2_daddr_t size)
{
	int rv;

	ERROR(disk, NULL);
	rv = ufs_disk_write(disk);
	if (rv == -1) {
		ERROR(disk, "failed to open disk for writing");
		return(rv);
	}
	return (berase_helper(disk, blockno, size));
}