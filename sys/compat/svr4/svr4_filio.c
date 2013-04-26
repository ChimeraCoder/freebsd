
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
#include <sys/proc.h>
#include <sys/systm.h>
#include <sys/capability.h>
#include <sys/file.h>
#include <sys/filio.h>
#include <sys/lock.h>
#include <sys/signal.h>
#include <sys/filedesc.h>
#include <sys/poll.h>
#include <sys/malloc.h>
#include <sys/mutex.h>

#include <sys/sysproto.h>

#include <compat/svr4/svr4.h>
#include <compat/svr4/svr4_types.h>
#include <compat/svr4/svr4_util.h>
#include <compat/svr4/svr4_signal.h>
#include <compat/svr4/svr4_proto.h>
#include <compat/svr4/svr4_ioctl.h>
#include <compat/svr4/svr4_filio.h>

/*#define GROTTY_READ_HACK*/

int
svr4_sys_poll(td, uap)
     struct thread *td;
     struct svr4_sys_poll_args *uap;
{
     int error;
     struct poll_args pa;
     struct pollfd *pfd;
     int idx = 0, cerr;
     u_long siz;

     if (uap->nfds > maxfilesperproc && uap->nfds > FD_SETSIZE)
       return (EINVAL);

     pa.fds = uap->fds;
     pa.nfds = uap->nfds;
     pa.timeout = uap->timeout;

     siz = uap->nfds * sizeof(struct pollfd);
     pfd = (struct pollfd *)malloc(siz, M_TEMP, M_WAITOK);

     error = sys_poll(td, (struct poll_args *)uap);

     if ((cerr = copyin(uap->fds, pfd, siz)) != 0) {
       error = cerr;
       goto done;
     }

     for (idx = 0; idx < uap->nfds; idx++) {
       /* POLLWRNORM already equals POLLOUT, so we don't worry about that */
       if (pfd[idx].revents & (POLLOUT | POLLWRNORM | POLLWRBAND))
	    pfd[idx].revents |= (POLLOUT | POLLWRNORM | POLLWRBAND);
     }
     if ((cerr = copyout(pfd, uap->fds, siz)) != 0) {
       error = cerr;
       goto done;   /* yeah, I know it's the next line, but this way I won't
		       forget to update it if I add more code */
     }
done:
     free(pfd, M_TEMP);
     return error;
}

#if defined(READ_TEST)
int
svr4_sys_read(td, uap)
     struct thread *td;
     struct svr4_sys_read_args *uap;
{
     struct read_args ra;
     struct file *fp;
     struct socket *so = NULL;
     int so_state;
     sigset_t sigmask;
     int rv;

     ra.fd = uap->fd;
     ra.buf = uap->buf;
     ra.nbyte = uap->nbyte;

     if (fget(td, uap->fd, CAP_READ, &fp) != 0) {
       DPRINTF(("Something fishy with the user-supplied file descriptor...\n"));
       return EBADF;
     }

     if (fp->f_type == DTYPE_SOCKET) {
       so = fp->f_data;
       DPRINTF(("fd %d is a socket\n", uap->fd));
       if (so->so_state & SS_ASYNC) {
	 DPRINTF(("fd %d is an ASYNC socket!\n", uap->fd));
       }
       DPRINTF(("Here are its flags: 0x%x\n", so->so_state));
#if defined(GROTTY_READ_HACK)
       so_state = so->so_state;
       so->so_state &= ~SS_NBIO;
#endif
     }

     rv = read(td, &ra);

     DPRINTF(("svr4_read(%d, 0x%0x, %d) = %d\n", 
	     uap->fd, uap->buf, uap->nbyte, rv));
     if (rv == EAGAIN) {
#ifdef DEBUG_SVR4
       struct sigacts *ps;

       PROC_LOCK(td->td_proc);
       ps = td->td_proc->p_sigacts;
       mtx_lock(&ps->ps_mtx);
#endif
       DPRINTF(("sigmask = 0x%x\n", td->td_sigmask));
       DPRINTF(("sigignore = 0x%x\n", ps->ps_sigignore));
       DPRINTF(("sigcaught = 0x%x\n", ps->ps_sigcatch));
       DPRINTF(("siglist = 0x%x\n", td->td_siglist));
#ifdef DEBUG_SVR4
       mtx_unlock(&ps->ps_mtx);
       PROC_UNLOCK(td->td_proc);
#endif
     }

#if defined(GROTTY_READ_HACK)
     if (so) {  /* We've already checked to see if this is a socket */
       so->so_state = so_state;
     }
#endif
     fdrop(fp, td);

     return(rv);
}
#endif /* READ_TEST */

#if defined(BOGUS)
int
svr4_sys_write(td, uap)
     struct thread *td;
     struct svr4_sys_write_args *uap;
{
     struct write_args wa;
     struct file *fp;
     int rv;

     wa.fd = uap->fd;
     wa.buf = uap->buf;
     wa.nbyte = uap->nbyte;

     rv = write(td, &wa);

     DPRINTF(("svr4_write(%d, 0x%0x, %d) = %d\n", 
	     uap->fd, uap->buf, uap->nbyte, rv));

     return(rv);
}
#endif /* BOGUS */

int
svr4_fil_ioctl(fp, td, retval, fd, cmd, data)
	struct file *fp;
	struct thread *td;
	register_t *retval;
	int fd;
	u_long cmd;
	caddr_t data;
{
	struct filedesc *fdp = td->td_proc->p_fd;
	struct filedescent *fde;
	int error, num;

	*retval = 0;

	switch (cmd) {
	case SVR4_FIOCLEX:
		FILEDESC_XLOCK(fdp);
		fde = &fdp->fd_ofiles[fd];
		fde->fde_flags |= UF_EXCLOSE;
		FILEDESC_XUNLOCK(fdp);
		return 0;

	case SVR4_FIONCLEX:
		FILEDESC_XLOCK(fdp);
		fde = &fdp->fd_ofiles[fd];
		fde->fde_flags &= ~UF_EXCLOSE;
		FILEDESC_XUNLOCK(fdp);
		return 0;

	case SVR4_FIOGETOWN:
	case SVR4_FIOSETOWN:
	case SVR4_FIOASYNC:
	case SVR4_FIONBIO:
	case SVR4_FIONREAD:
		if ((error = copyin(data, &num, sizeof(num))) != 0)
			return error;

		switch (cmd) {
		case SVR4_FIOGETOWN:	cmd = FIOGETOWN; break;
		case SVR4_FIOSETOWN:	cmd = FIOSETOWN; break;
		case SVR4_FIOASYNC:	cmd = FIOASYNC;  break;
		case SVR4_FIONBIO:	cmd = FIONBIO;   break;
		case SVR4_FIONREAD:	cmd = FIONREAD;  break;
		}

#ifdef SVR4_DEBUG
		if (cmd == FIOASYNC) DPRINTF(("FIOASYNC\n"));
#endif
		error = fo_ioctl(fp, cmd, (caddr_t) &num, td->td_ucred, td);

		if (error)
			return error;

		return copyout(&num, data, sizeof(num));

	default:
		DPRINTF(("Unknown svr4 filio %lx\n", cmd));
		return 0;	/* ENOSYS really */
	}
}