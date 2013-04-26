
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

#include "opt_compat.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/namei.h>
#include <sys/filedesc.h>
#include <sys/kernel.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/syscallsubr.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/proc.h>
#include <sys/uio.h>
#include <sys/malloc.h>
#include <sys/dirent.h>
#include <ufs/ufs/dir.h>	/* XXX only for DIRBLKSIZ */

#ifdef COMPAT_LINUX32
#include <machine/../linux32/linux.h>
#include <machine/../linux32/linux32_proto.h>
#else
#include <machine/../linux/linux.h>
#include <machine/../linux/linux_proto.h>
#endif
#include <compat/linux/linux_util.h>

#include <security/mac/mac_framework.h>

static int
linux_getcwd_scandir(struct vnode **, struct vnode **,
    char **, char *, struct thread *);
static int
linux_getcwd_common(struct vnode *, struct vnode *,
		   char **, char *, int, int, struct thread *);

#define DIRENT_MINSIZE (sizeof(struct dirent) - (MAXNAMLEN+1) + 4)

/*
 * Vnode variable naming conventions in this file:
 *
 * rvp: the current root we're aiming towards.
 * lvp, *lvpp: the "lower" vnode
 * uvp, *uvpp: the "upper" vnode.
 *
 * Since all the vnodes we're dealing with are directories, and the
 * lookups are going *up* in the filesystem rather than *down*, the
 * usual "pvp" (parent) or "dvp" (directory) naming conventions are
 * too confusing.
 */

/*
 * XXX Will infinite loop in certain cases if a directory read reliably
 *	returns EINVAL on last block.
 * XXX is EINVAL the right thing to return if a directory is malformed?
 */

/*
 * XXX Untested vs. mount -o union; probably does the wrong thing.
 */

/*
 * Find parent vnode of *lvpp, return in *uvpp
 *
 * If we care about the name, scan it looking for name of directory
 * entry pointing at lvp.
 *
 * Place the name in the buffer which starts at bufp, immediately
 * before *bpp, and move bpp backwards to point at the start of it.
 *
 * On entry, *lvpp is a locked vnode reference; on exit, it is vput and NULL'ed
 * On exit, *uvpp is either NULL or is a locked vnode reference.
 */
static int
linux_getcwd_scandir(lvpp, uvpp, bpp, bufp, td)
	struct vnode **lvpp;
	struct vnode **uvpp;
	char **bpp;
	char *bufp;
	struct thread *td;
{
	int     error = 0;
	int     eofflag;
	off_t   off;
	int     tries;
	struct uio uio;
	struct iovec iov;
	char   *dirbuf = NULL;
	int	dirbuflen;
	ino_t   fileno;
	struct vattr va;
	struct vnode *uvp = NULL;
	struct vnode *lvp = *lvpp;	
	struct componentname cn;
	int len, reclen;
	tries = 0;

	/*
	 * If we want the filename, get some info we need while the
	 * current directory is still locked.
	 */
	if (bufp != NULL) {
		error = VOP_GETATTR(lvp, &va, td->td_ucred);
		if (error) {
			vput(lvp);
			*lvpp = NULL;
			*uvpp = NULL;
			return error;
		}
	}

	/*
	 * Ok, we have to do it the hard way..
	 * Next, get parent vnode using lookup of ..
	 */
	cn.cn_nameiop = LOOKUP;
	cn.cn_flags = ISLASTCN | ISDOTDOT | RDONLY;
	cn.cn_thread = td;
	cn.cn_cred = td->td_ucred;
	cn.cn_pnbuf = NULL;
	cn.cn_nameptr = "..";
	cn.cn_namelen = 2;
	cn.cn_consume = 0;
	cn.cn_lkflags = LK_SHARED;
	
	/*
	 * At this point, lvp is locked and will be unlocked by the lookup.
	 * On successful return, *uvpp will be locked
	 */
#ifdef MAC
	error = mac_vnode_check_lookup(td->td_ucred, lvp, &cn);
	if (error == 0)
#endif
		error = VOP_LOOKUP(lvp, uvpp, &cn);
	if (error) {
		vput(lvp);
		*lvpp = NULL;
		*uvpp = NULL;
		return error;
	}
	uvp = *uvpp;

	/* If we don't care about the pathname, we're done */
	if (bufp == NULL) {
		vput(lvp);
		*lvpp = NULL;
		return 0;
	}
	
	fileno = va.va_fileid;

	dirbuflen = DIRBLKSIZ;
	if (dirbuflen < va.va_blocksize)
		dirbuflen = va.va_blocksize;
	dirbuf = (char *)malloc(dirbuflen, M_TEMP, M_WAITOK);

#if 0
unionread:
#endif
	off = 0;
	do {
		/* call VOP_READDIR of parent */
		iov.iov_base = dirbuf;
		iov.iov_len = dirbuflen;

		uio.uio_iov = &iov;
		uio.uio_iovcnt = 1;
		uio.uio_offset = off;
		uio.uio_resid = dirbuflen;
		uio.uio_segflg = UIO_SYSSPACE;
		uio.uio_rw = UIO_READ;
		uio.uio_td = td;

		eofflag = 0;

#ifdef MAC
		error = mac_vnode_check_readdir(td->td_ucred, uvp);
		if (error == 0)
#endif /* MAC */
			error = VOP_READDIR(uvp, &uio, td->td_ucred, &eofflag,
			    0, 0);

		off = uio.uio_offset;

		/*
		 * Try again if NFS tosses its cookies.
		 * XXX this can still loop forever if the directory is busted
		 * such that the second or subsequent page of it always
		 * returns EINVAL
		 */
		if ((error == EINVAL) && (tries < 3)) {
			off = 0;
			tries++;
			continue;	/* once more, with feeling */
		}

		if (!error) {
			char   *cpos;
			struct dirent *dp;
			
			cpos = dirbuf;
			tries = 0;
				
			/* scan directory page looking for matching vnode */ 
			for (len = (dirbuflen - uio.uio_resid); len > 0; len -= reclen) {
				dp = (struct dirent *) cpos;
				reclen = dp->d_reclen;

				/* check for malformed directory.. */
				if (reclen < DIRENT_MINSIZE) {
					error = EINVAL;
					goto out;
				}
				/*
				 * XXX should perhaps do VOP_LOOKUP to
				 * check that we got back to the right place,
				 * but getting the locking games for that
				 * right would be heinous.
				 */
				if ((dp->d_type != DT_WHT) &&
				    (dp->d_fileno == fileno)) {
					char *bp = *bpp;
					bp -= dp->d_namlen;
					
					if (bp <= bufp) {
						error = ERANGE;
						goto out;
					}
					bcopy(dp->d_name, bp, dp->d_namlen);
					error = 0;
					*bpp = bp;
					goto out;
				}
				cpos += reclen;
			}
		}
	} while (!eofflag);
	error = ENOENT;
		
out:
	vput(lvp);
	*lvpp = NULL;
	free(dirbuf, M_TEMP);
	return error;
}


/*
 * common routine shared by sys___getcwd() and linux_vn_isunder()
 */

#define GETCWD_CHECK_ACCESS 0x0001

static int
linux_getcwd_common (lvp, rvp, bpp, bufp, limit, flags, td)
	struct vnode *lvp;
	struct vnode *rvp;
	char **bpp;
	char *bufp;
	int limit;
	int flags;
	struct thread *td;
{
	struct filedesc *fdp = td->td_proc->p_fd;
	struct vnode *uvp = NULL;
	char *bp = NULL;
	int error;
	accmode_t accmode = VEXEC;

	if (rvp == NULL) {
		rvp = fdp->fd_rdir;
		if (rvp == NULL)
			rvp = rootvnode;
	}
	
	VREF(rvp);
	VREF(lvp);

	/*
	 * Error handling invariant:
	 * Before a `goto out':
	 *	lvp is either NULL, or locked and held.
	 *	uvp is either NULL, or locked and held.
	 */

	error = vn_lock(lvp, LK_EXCLUSIVE | LK_RETRY);
	if (error != 0)
		panic("vn_lock LK_RETRY returned error %d", error);
	if (bufp)
		bp = *bpp;
	/*
	 * this loop will terminate when one of the following happens:
	 *	- we hit the root
	 *	- getdirentries or lookup fails
	 *	- we run out of space in the buffer.
	 */
	if (lvp == rvp) {
		if (bp)
			*(--bp) = '/';
		goto out;
	}
	do {
		if (lvp->v_type != VDIR) {
			error = ENOTDIR;
			goto out;
		}
		
		/*
		 * access check here is optional, depending on
		 * whether or not caller cares.
		 */
		if (flags & GETCWD_CHECK_ACCESS) {
			error = VOP_ACCESS(lvp, accmode, td->td_ucred, td);
			if (error)
				goto out;
			accmode = VEXEC|VREAD;
		}
		
		/*
		 * step up if we're a covered vnode..
		 */
		while (lvp->v_vflag & VV_ROOT) {
			struct vnode *tvp;

			if (lvp == rvp)
				goto out;
			
			tvp = lvp;
			lvp = lvp->v_mount->mnt_vnodecovered;
			vput(tvp);
			/*
			 * hodie natus est radici frater
			 */
			if (lvp == NULL) {
				error = ENOENT;
				goto out;
			}
			VREF(lvp);
			error = vn_lock(lvp, LK_EXCLUSIVE | LK_RETRY);
			if (error != 0)
				panic("vn_lock LK_RETRY returned %d", error);
		}
		error = linux_getcwd_scandir(&lvp, &uvp, &bp, bufp, td);
		if (error)
			goto out;
#ifdef DIAGNOSTIC		
		if (lvp != NULL)
			panic("getcwd: oops, forgot to null lvp");
		if (bufp && (bp <= bufp)) {
			panic("getcwd: oops, went back too far");
		}
#endif		
		if (bp) 
			*(--bp) = '/';
		lvp = uvp;
		uvp = NULL;
		limit--;
	} while ((lvp != rvp) && (limit > 0)); 

out:
	if (bpp)
		*bpp = bp;
	if (uvp)
		vput(uvp);
	if (lvp)
		vput(lvp);
	vrele(rvp);
	return error;
}


/*
 * Find pathname of process's current directory.
 *
 * Use vfs vnode-to-name reverse cache; if that fails, fall back
 * to reading directory contents.
 */

int
linux_getcwd(struct thread *td, struct linux_getcwd_args *args)
{
	caddr_t bp, bend, path;
	int error, len, lenused;

#ifdef DEBUG
	if (ldebug(getcwd))
		printf(ARGS(getcwd, "%p, %ld"), args->buf, (long)args->bufsize);
#endif

	len = args->bufsize;

	if (len > MAXPATHLEN*4)
		len = MAXPATHLEN*4;
	else if (len < 2)
		return ERANGE;

	path = (char *)malloc(len, M_TEMP, M_WAITOK);

	error = kern___getcwd(td, path, UIO_SYSSPACE, len);
	if (!error) {
		lenused = strlen(path) + 1;
		if (lenused <= args->bufsize) {
			td->td_retval[0] = lenused;
			error = copyout(path, args->buf, lenused);
		}
		else
			error = ERANGE;
	} else {
		bp = &path[len];
		bend = bp;
		*(--bp) = '\0';

		/*
		 * 5th argument here is "max number of vnodes to traverse".
		 * Since each entry takes up at least 2 bytes in the output buffer,
		 * limit it to N/2 vnodes for an N byte buffer.
		 */

		mtx_lock(&Giant);
		error = linux_getcwd_common (td->td_proc->p_fd->fd_cdir, NULL,
		    &bp, path, len/2, GETCWD_CHECK_ACCESS, td);
		mtx_unlock(&Giant);

		if (error)
			goto out;
		lenused = bend - bp;
		td->td_retval[0] = lenused;
		/* put the result into user buffer */
		error = copyout(bp, args->buf, lenused);
	}
out:
	free(path, M_TEMP);
	return (error);
}