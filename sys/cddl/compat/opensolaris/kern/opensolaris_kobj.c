
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

#include <sys/types.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/kthread.h>
#include <sys/namei.h>
#include <sys/proc.h>
#include <sys/filedesc.h>
#include <sys/fcntl.h>
#include <sys/linker.h>
#include <sys/kobj.h>

void
kobj_free(void *address, size_t size)
{

	kmem_free(address, size);
}

void *
kobj_alloc(size_t size, int flag)
{

	return (kmem_alloc(size, (flag & KM_NOWAIT) ? KM_NOSLEEP : KM_SLEEP));
}

void *
kobj_zalloc(size_t size, int flag)
{
	void *p;

	if ((p = kobj_alloc(size, flag)) != NULL)
		bzero(p, size);
	return (p);
}

static void *
kobj_open_file_vnode(const char *file)
{
	struct thread *td = curthread;
	struct filedesc *fd;
	struct nameidata nd;
	int error, flags;

	fd = td->td_proc->p_fd;
	FILEDESC_XLOCK(fd);
	if (fd->fd_rdir == NULL) {
		fd->fd_rdir = rootvnode;
		vref(fd->fd_rdir);
	}
	if (fd->fd_cdir == NULL) {
		fd->fd_cdir = rootvnode;
		vref(fd->fd_cdir);
	}
	FILEDESC_XUNLOCK(fd);

	flags = FREAD | O_NOFOLLOW;
	NDINIT(&nd, LOOKUP, 0, UIO_SYSSPACE, file, td);
	error = vn_open_cred(&nd, &flags, 0, 0, curthread->td_ucred, NULL);
	if (error != 0)
		return (NULL);
	NDFREE(&nd, NDF_ONLY_PNBUF);
	/* We just unlock so we hold a reference. */
	VOP_UNLOCK(nd.ni_vp, 0);
	return (nd.ni_vp);
}

static void *
kobj_open_file_loader(const char *file)
{

	return (preload_search_by_name(file));
}

struct _buf *
kobj_open_file(const char *file)
{
	struct _buf *out;

	out = kmem_alloc(sizeof(*out), KM_SLEEP);
	out->mounted = root_mounted();
	/*
	 * If root is already mounted we read file using file system,
	 * if not, we use loader.
	 */
	if (out->mounted)
		out->ptr = kobj_open_file_vnode(file);
	else
		out->ptr = kobj_open_file_loader(file);
	if (out->ptr == NULL) {
		kmem_free(out, sizeof(*out));
		return ((struct _buf *)-1);
	}
	return (out);
}

static int
kobj_get_filesize_vnode(struct _buf *file, uint64_t *size)
{
	struct vnode *vp = file->ptr;
	struct vattr va;
	int error;

	vn_lock(vp, LK_SHARED | LK_RETRY);
	error = VOP_GETATTR(vp, &va, curthread->td_ucred);
	VOP_UNLOCK(vp, 0);
	if (error == 0)
		*size = (uint64_t)va.va_size;
	return (error);
}

static int
kobj_get_filesize_loader(struct _buf *file, uint64_t *size)
{
	void *ptr;

	ptr = preload_search_info(file->ptr, MODINFO_SIZE);
	if (ptr == NULL)
		return (ENOENT);
	*size = (uint64_t)*(size_t *)ptr;
	return (0);
}

int
kobj_get_filesize(struct _buf *file, uint64_t *size)
{

	if (file->mounted)
		return (kobj_get_filesize_vnode(file, size));
	else
		return (kobj_get_filesize_loader(file, size));
}

int
kobj_read_file_vnode(struct _buf *file, char *buf, unsigned size, unsigned off)
{
	struct vnode *vp = file->ptr;
	struct thread *td = curthread;
	struct uio auio;
	struct iovec aiov;
	int error;

	bzero(&aiov, sizeof(aiov));
	bzero(&auio, sizeof(auio));

	aiov.iov_base = buf;
	aiov.iov_len = size;

	auio.uio_iov = &aiov;
	auio.uio_offset = (off_t)off;
	auio.uio_segflg = UIO_SYSSPACE;
	auio.uio_rw = UIO_READ;
	auio.uio_iovcnt = 1;
	auio.uio_resid = size;
	auio.uio_td = td;

	vn_lock(vp, LK_SHARED | LK_RETRY);
	error = VOP_READ(vp, &auio, IO_UNIT | IO_SYNC, td->td_ucred);
	VOP_UNLOCK(vp, 0);
	return (error != 0 ? -1 : size - auio.uio_resid);
}

int
kobj_read_file_loader(struct _buf *file, char *buf, unsigned size, unsigned off)
{
	char *ptr;

	ptr = preload_fetch_addr(file->ptr);
	if (ptr == NULL)
		return (ENOENT);
	bcopy(ptr + off, buf, size);
	return (0);
}

int
kobj_read_file(struct _buf *file, char *buf, unsigned size, unsigned off)
{

	if (file->mounted)
		return (kobj_read_file_vnode(file, buf, size, off));
	else
		return (kobj_read_file_loader(file, buf, size, off));
}

void
kobj_close_file(struct _buf *file)
{

	if (file->mounted)
		vn_close(file->ptr, FREAD, curthread->td_ucred, curthread);
	kmem_free(file, sizeof(*file));
}