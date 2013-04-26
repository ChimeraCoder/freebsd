
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
#include <sys/capability.h>
#include <sys/file.h>
#include <sys/filedesc.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/systm.h>

#include <compat/svr4/svr4.h>
#include <compat/svr4/svr4_types.h>
#include <compat/svr4/svr4_util.h>
#include <compat/svr4/svr4_signal.h>
#include <compat/svr4/svr4_proto.h>
#include <compat/svr4/svr4_stropts.h>
#include <compat/svr4/svr4_ioctl.h>
#include <compat/svr4/svr4_termios.h>
#include <compat/svr4/svr4_filio.h>
#include <compat/svr4/svr4_sockio.h>

#ifdef DEBUG_SVR4
static void svr4_decode_cmd(u_long, char *, char *, int *, int *);
/*
 * Decode an ioctl command symbolically
 */
static void
svr4_decode_cmd(cmd, dir, c, num, argsiz)
	u_long		  cmd;
	char		 *dir, *c;
	int		 *num, *argsiz;
{
	if (cmd & SVR4_IOC_VOID)
		*dir++ = 'V';
	if (cmd & SVR4_IOC_IN)
		*dir++ = 'R';
	if (cmd & SVR4_IOC_OUT)
		*dir++ = 'W';
	*dir = '\0';
	if (cmd & SVR4_IOC_INOUT)
		*argsiz = (cmd >> 16) & 0xff;
	else
		*argsiz = -1;

	*c = (cmd >> 8) & 0xff;
	*num = cmd & 0xff;
}
#endif

int
svr4_sys_ioctl(td, uap)
	struct thread *td;
	struct svr4_sys_ioctl_args *uap;
{
	int             *retval;
	struct file	*fp;
	u_long		 cmd;
	int (*fun)(struct file *, struct thread *, register_t *,
			int, u_long, caddr_t);
	int error;
#ifdef DEBUG_SVR4
	char		 dir[4];
	char		 c;
	int		 num;
	int		 argsiz;

	svr4_decode_cmd(uap->com, dir, &c, &num, &argsiz);

	DPRINTF(("svr4_ioctl[%lx](%d, _IO%s(%c, %d, %d), %p);\n", uap->com, uap->fd,
	    dir, c, num, argsiz, uap->data));
#endif
	retval = td->td_retval;
	cmd = uap->com;

	if ((error = fget(td, uap->fd, CAP_IOCTL, &fp)) != 0)
		return (error);

	if ((fp->f_flag & (FREAD | FWRITE)) == 0) {
		fdrop(fp, td);
		return EBADF;
	}

#if defined(DEBUG_SVR4)
	if (fp->f_type == DTYPE_SOCKET) {
	        struct socket *so = fp->f_data;
		DPRINTF(("<<< IN: so_state = 0x%x\n", so->so_state));
	}
#endif

	switch (cmd & 0xff00) {
	case SVR4_TIOC:
	        DPRINTF(("term\n"));
		fun = svr4_term_ioctl;
		break;

	case SVR4_STR:
	        DPRINTF(("stream\n"));
		fun = svr4_stream_ioctl;
		break;

	case SVR4_FIOC:
                DPRINTF(("file\n"));
		fun = svr4_fil_ioctl;
		break;

	case SVR4_SIOC:
	        DPRINTF(("socket\n"));
		fun = svr4_sock_ioctl;
		break;

	case SVR4_XIOC:
		/* We do not support those */
		fdrop(fp, td);
		return EINVAL;

	default:
		fdrop(fp, td);
		DPRINTF(("Unimplemented ioctl %lx\n", cmd));
		return 0;	/* XXX: really ENOSYS */
	}
#if defined(DEBUG_SVR4)
	if (fp->f_type == DTYPE_SOCKET) {
	        struct socket *so;

	        so = fp->f_data;
		DPRINTF((">>> OUT: so_state = 0x%x\n", so->so_state));
	}
#endif
	error = (*fun)(fp, td, retval, uap->fd, cmd, uap->data);
	fdrop(fp, td);
	return (error);
}