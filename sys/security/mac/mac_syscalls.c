
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

#include "opt_mac.h"

#include <sys/param.h>
#include <sys/capability.h>
#include <sys/fcntl.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/mac.h>
#include <sys/proc.h>
#include <sys/systm.h>
#include <sys/sysctl.h>
#include <sys/sysproto.h>
#include <sys/sysent.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/file.h>
#include <sys/namei.h>
#include <sys/socket.h>
#include <sys/pipe.h>
#include <sys/socketvar.h>

#include <security/mac/mac_framework.h>
#include <security/mac/mac_internal.h>
#include <security/mac/mac_policy.h>

#ifdef MAC

FEATURE(security_mac, "Mandatory Access Control Framework support");

int
sys___mac_get_pid(struct thread *td, struct __mac_get_pid_args *uap)
{
	char *elements, *buffer;
	struct mac mac;
	struct proc *tproc;
	struct ucred *tcred;
	int error;

	error = copyin(uap->mac_p, &mac, sizeof(mac));
	if (error)
		return (error);

	error = mac_check_structmac_consistent(&mac);
	if (error)
		return (error);

	tproc = pfind(uap->pid);
	if (tproc == NULL)
		return (ESRCH);

	tcred = NULL;				/* Satisfy gcc. */
	error = p_cansee(td, tproc);
	if (error == 0)
		tcred = crhold(tproc->p_ucred);
	PROC_UNLOCK(tproc);
	if (error)
		return (error);

	elements = malloc(mac.m_buflen, M_MACTEMP, M_WAITOK);
	error = copyinstr(mac.m_string, elements, mac.m_buflen, NULL);
	if (error) {
		free(elements, M_MACTEMP);
		crfree(tcred);
		return (error);
	}

	buffer = malloc(mac.m_buflen, M_MACTEMP, M_WAITOK | M_ZERO);
	error = mac_cred_externalize_label(tcred->cr_label, elements,
	    buffer, mac.m_buflen);
	if (error == 0)
		error = copyout(buffer, mac.m_string, strlen(buffer)+1);

	free(buffer, M_MACTEMP);
	free(elements, M_MACTEMP);
	crfree(tcred);
	return (error);
}

int
sys___mac_get_proc(struct thread *td, struct __mac_get_proc_args *uap)
{
	char *elements, *buffer;
	struct mac mac;
	int error;

	error = copyin(uap->mac_p, &mac, sizeof(mac));
	if (error)
		return (error);

	error = mac_check_structmac_consistent(&mac);
	if (error)
		return (error);

	elements = malloc(mac.m_buflen, M_MACTEMP, M_WAITOK);
	error = copyinstr(mac.m_string, elements, mac.m_buflen, NULL);
	if (error) {
		free(elements, M_MACTEMP);
		return (error);
	}

	buffer = malloc(mac.m_buflen, M_MACTEMP, M_WAITOK | M_ZERO);
	error = mac_cred_externalize_label(td->td_ucred->cr_label,
	    elements, buffer, mac.m_buflen);
	if (error == 0)
		error = copyout(buffer, mac.m_string, strlen(buffer)+1);

	free(buffer, M_MACTEMP);
	free(elements, M_MACTEMP);
	return (error);
}

int
sys___mac_set_proc(struct thread *td, struct __mac_set_proc_args *uap)
{
	struct ucred *newcred, *oldcred;
	struct label *intlabel;
	struct proc *p;
	struct mac mac;
	char *buffer;
	int error;

	if (!(mac_labeled & MPC_OBJECT_CRED))
		return (EINVAL);

	error = copyin(uap->mac_p, &mac, sizeof(mac));
	if (error)
		return (error);

	error = mac_check_structmac_consistent(&mac);
	if (error)
		return (error);

	buffer = malloc(mac.m_buflen, M_MACTEMP, M_WAITOK);
	error = copyinstr(mac.m_string, buffer, mac.m_buflen, NULL);
	if (error) {
		free(buffer, M_MACTEMP);
		return (error);
	}

	intlabel = mac_cred_label_alloc();
	error = mac_cred_internalize_label(intlabel, buffer);
	free(buffer, M_MACTEMP);
	if (error)
		goto out;

	newcred = crget();

	p = td->td_proc;
	PROC_LOCK(p);
	oldcred = p->p_ucred;

	error = mac_cred_check_relabel(oldcred, intlabel);
	if (error) {
		PROC_UNLOCK(p);
		crfree(newcred);
		goto out;
	}

	setsugid(p);
	crcopy(newcred, oldcred);
	mac_cred_relabel(newcred, intlabel);
	p->p_ucred = newcred;

	PROC_UNLOCK(p);
	crfree(oldcred);
	mac_proc_vm_revoke(td);

out:
	mac_cred_label_free(intlabel);
	return (error);
}

int
sys___mac_get_fd(struct thread *td, struct __mac_get_fd_args *uap)
{
	char *elements, *buffer;
	struct label *intlabel;
	struct file *fp;
	struct mac mac;
	struct vnode *vp;
	struct pipe *pipe;
	struct socket *so;
	short label_type;
	int error;

	error = copyin(uap->mac_p, &mac, sizeof(mac));
	if (error)
		return (error);

	error = mac_check_structmac_consistent(&mac);
	if (error)
		return (error);

	elements = malloc(mac.m_buflen, M_MACTEMP, M_WAITOK);
	error = copyinstr(mac.m_string, elements, mac.m_buflen, NULL);
	if (error) {
		free(elements, M_MACTEMP);
		return (error);
	}

	buffer = malloc(mac.m_buflen, M_MACTEMP, M_WAITOK | M_ZERO);
	error = fget(td, uap->fd, CAP_MAC_GET, &fp);
	if (error)
		goto out;

	label_type = fp->f_type;
	switch (fp->f_type) {
	case DTYPE_FIFO:
	case DTYPE_VNODE:
		if (!(mac_labeled & MPC_OBJECT_VNODE)) {
			error = EINVAL;
			goto out_fdrop;
		}
		vp = fp->f_vnode;
		intlabel = mac_vnode_label_alloc();
		vn_lock(vp, LK_EXCLUSIVE | LK_RETRY);
		mac_vnode_copy_label(vp->v_label, intlabel);
		VOP_UNLOCK(vp, 0);
		error = mac_vnode_externalize_label(intlabel, elements,
		    buffer, mac.m_buflen);
		mac_vnode_label_free(intlabel);
		break;

	case DTYPE_PIPE:
		if (!(mac_labeled & MPC_OBJECT_PIPE)) {
			error = EINVAL;
			goto out_fdrop;
		}
		pipe = fp->f_data;
		intlabel = mac_pipe_label_alloc();
		PIPE_LOCK(pipe);
		mac_pipe_copy_label(pipe->pipe_pair->pp_label, intlabel);
		PIPE_UNLOCK(pipe);
		error = mac_pipe_externalize_label(intlabel, elements,
		    buffer, mac.m_buflen);
		mac_pipe_label_free(intlabel);
		break;

	case DTYPE_SOCKET:
		if (!(mac_labeled & MPC_OBJECT_SOCKET)) {
			error = EINVAL;
			goto out_fdrop;
		}
		so = fp->f_data;
		intlabel = mac_socket_label_alloc(M_WAITOK);
		SOCK_LOCK(so);
		mac_socket_copy_label(so->so_label, intlabel);
		SOCK_UNLOCK(so);
		error = mac_socket_externalize_label(intlabel, elements,
		    buffer, mac.m_buflen);
		mac_socket_label_free(intlabel);
		break;

	default:
		error = EINVAL;
	}
	if (error == 0)
		error = copyout(buffer, mac.m_string, strlen(buffer)+1);
out_fdrop:
	fdrop(fp, td);
out:
	free(buffer, M_MACTEMP);
	free(elements, M_MACTEMP);
	return (error);
}

int
sys___mac_get_file(struct thread *td, struct __mac_get_file_args *uap)
{
	char *elements, *buffer;
	struct nameidata nd;
	struct label *intlabel;
	struct mac mac;
	int error;

	if (!(mac_labeled & MPC_OBJECT_VNODE))
		return (EINVAL);

	error = copyin(uap->mac_p, &mac, sizeof(mac));
	if (error)
		return (error);

	error = mac_check_structmac_consistent(&mac);
	if (error)
		return (error);

	elements = malloc(mac.m_buflen, M_MACTEMP, M_WAITOK);
	error = copyinstr(mac.m_string, elements, mac.m_buflen, NULL);
	if (error) {
		free(elements, M_MACTEMP);
		return (error);
	}

	buffer = malloc(mac.m_buflen, M_MACTEMP, M_WAITOK | M_ZERO);
	NDINIT(&nd, LOOKUP, LOCKLEAF | FOLLOW, UIO_USERSPACE,
	    uap->path_p, td);
	error = namei(&nd);
	if (error)
		goto out;

	intlabel = mac_vnode_label_alloc();
	mac_vnode_copy_label(nd.ni_vp->v_label, intlabel);
	error = mac_vnode_externalize_label(intlabel, elements, buffer,
	    mac.m_buflen);

	NDFREE(&nd, 0);
	mac_vnode_label_free(intlabel);
	if (error == 0)
		error = copyout(buffer, mac.m_string, strlen(buffer)+1);

out:
	free(buffer, M_MACTEMP);
	free(elements, M_MACTEMP);

	return (error);
}

int
sys___mac_get_link(struct thread *td, struct __mac_get_link_args *uap)
{
	char *elements, *buffer;
	struct nameidata nd;
	struct label *intlabel;
	struct mac mac;
	int error;

	if (!(mac_labeled & MPC_OBJECT_VNODE))
		return (EINVAL);

	error = copyin(uap->mac_p, &mac, sizeof(mac));
	if (error)
		return (error);

	error = mac_check_structmac_consistent(&mac);
	if (error)
		return (error);

	elements = malloc(mac.m_buflen, M_MACTEMP, M_WAITOK);
	error = copyinstr(mac.m_string, elements, mac.m_buflen, NULL);
	if (error) {
		free(elements, M_MACTEMP);
		return (error);
	}

	buffer = malloc(mac.m_buflen, M_MACTEMP, M_WAITOK | M_ZERO);
	NDINIT(&nd, LOOKUP, LOCKLEAF | NOFOLLOW, UIO_USERSPACE,
	    uap->path_p, td);
	error = namei(&nd);
	if (error)
		goto out;

	intlabel = mac_vnode_label_alloc();
	mac_vnode_copy_label(nd.ni_vp->v_label, intlabel);
	error = mac_vnode_externalize_label(intlabel, elements, buffer,
	    mac.m_buflen);
	NDFREE(&nd, 0);
	mac_vnode_label_free(intlabel);

	if (error == 0)
		error = copyout(buffer, mac.m_string, strlen(buffer)+1);

out:
	free(buffer, M_MACTEMP);
	free(elements, M_MACTEMP);

	return (error);
}

int
sys___mac_set_fd(struct thread *td, struct __mac_set_fd_args *uap)
{
	struct label *intlabel;
	struct pipe *pipe;
	struct socket *so;
	struct file *fp;
	struct mount *mp;
	struct vnode *vp;
	struct mac mac;
	char *buffer;
	int error;

	error = copyin(uap->mac_p, &mac, sizeof(mac));
	if (error)
		return (error);

	error = mac_check_structmac_consistent(&mac);
	if (error)
		return (error);

	buffer = malloc(mac.m_buflen, M_MACTEMP, M_WAITOK);
	error = copyinstr(mac.m_string, buffer, mac.m_buflen, NULL);
	if (error) {
		free(buffer, M_MACTEMP);
		return (error);
	}

	error = fget(td, uap->fd, CAP_MAC_SET, &fp);
	if (error)
		goto out;

	switch (fp->f_type) {
	case DTYPE_FIFO:
	case DTYPE_VNODE:
		if (!(mac_labeled & MPC_OBJECT_VNODE)) {
			error = EINVAL;
			goto out_fdrop;
		}
		intlabel = mac_vnode_label_alloc();
		error = mac_vnode_internalize_label(intlabel, buffer);
		if (error) {
			mac_vnode_label_free(intlabel);
			break;
		}
		vp = fp->f_vnode;
		error = vn_start_write(vp, &mp, V_WAIT | PCATCH);
		if (error != 0) {
			mac_vnode_label_free(intlabel);
			break;
		}
		vn_lock(vp, LK_EXCLUSIVE | LK_RETRY);
		error = vn_setlabel(vp, intlabel, td->td_ucred);
		VOP_UNLOCK(vp, 0);
		vn_finished_write(mp);
		mac_vnode_label_free(intlabel);
		break;

	case DTYPE_PIPE:
		if (!(mac_labeled & MPC_OBJECT_PIPE)) {
			error = EINVAL;
			goto out_fdrop;
		}
		intlabel = mac_pipe_label_alloc();
		error = mac_pipe_internalize_label(intlabel, buffer);
		if (error == 0) {
			pipe = fp->f_data;
			PIPE_LOCK(pipe);
			error = mac_pipe_label_set(td->td_ucred,
			    pipe->pipe_pair, intlabel);
			PIPE_UNLOCK(pipe);
		}
		mac_pipe_label_free(intlabel);
		break;

	case DTYPE_SOCKET:
		if (!(mac_labeled & MPC_OBJECT_SOCKET)) {
			error = EINVAL;
			goto out_fdrop;
		}
		intlabel = mac_socket_label_alloc(M_WAITOK);
		error = mac_socket_internalize_label(intlabel, buffer);
		if (error == 0) {
			so = fp->f_data;
			error = mac_socket_label_set(td->td_ucred, so,
			    intlabel);
		}
		mac_socket_label_free(intlabel);
		break;

	default:
		error = EINVAL;
	}
out_fdrop:
	fdrop(fp, td);
out:
	free(buffer, M_MACTEMP);
	return (error);
}

int
sys___mac_set_file(struct thread *td, struct __mac_set_file_args *uap)
{
	struct label *intlabel;
	struct nameidata nd;
	struct mount *mp;
	struct mac mac;
	char *buffer;
	int error;

	if (!(mac_labeled & MPC_OBJECT_VNODE))
		return (EINVAL);

	error = copyin(uap->mac_p, &mac, sizeof(mac));
	if (error)
		return (error);

	error = mac_check_structmac_consistent(&mac);
	if (error)
		return (error);

	buffer = malloc(mac.m_buflen, M_MACTEMP, M_WAITOK);
	error = copyinstr(mac.m_string, buffer, mac.m_buflen, NULL);
	if (error) {
		free(buffer, M_MACTEMP);
		return (error);
	}

	intlabel = mac_vnode_label_alloc();
	error = mac_vnode_internalize_label(intlabel, buffer);
	free(buffer, M_MACTEMP);
	if (error)
		goto out;

	NDINIT(&nd, LOOKUP, LOCKLEAF | FOLLOW, UIO_USERSPACE,
	    uap->path_p, td);
	error = namei(&nd);
	if (error == 0) {
		error = vn_start_write(nd.ni_vp, &mp, V_WAIT | PCATCH);
		if (error == 0) {
			error = vn_setlabel(nd.ni_vp, intlabel,
			    td->td_ucred);
			vn_finished_write(mp);
		}
	}

	NDFREE(&nd, 0);
out:
	mac_vnode_label_free(intlabel);
	return (error);
}

int
sys___mac_set_link(struct thread *td, struct __mac_set_link_args *uap)
{
	struct label *intlabel;
	struct nameidata nd;
	struct mount *mp;
	struct mac mac;
	char *buffer;
	int error;

	if (!(mac_labeled & MPC_OBJECT_VNODE))
		return (EINVAL);

	error = copyin(uap->mac_p, &mac, sizeof(mac));
	if (error)
		return (error);

	error = mac_check_structmac_consistent(&mac);
	if (error)
		return (error);

	buffer = malloc(mac.m_buflen, M_MACTEMP, M_WAITOK);
	error = copyinstr(mac.m_string, buffer, mac.m_buflen, NULL);
	if (error) {
		free(buffer, M_MACTEMP);
		return (error);
	}

	intlabel = mac_vnode_label_alloc();
	error = mac_vnode_internalize_label(intlabel, buffer);
	free(buffer, M_MACTEMP);
	if (error)
		goto out;

	NDINIT(&nd, LOOKUP, LOCKLEAF | NOFOLLOW, UIO_USERSPACE,
	    uap->path_p, td);
	error = namei(&nd);
	if (error == 0) {
		error = vn_start_write(nd.ni_vp, &mp, V_WAIT | PCATCH);
		if (error == 0) {
			error = vn_setlabel(nd.ni_vp, intlabel,
			    td->td_ucred);
			vn_finished_write(mp);
		}
	}

	NDFREE(&nd, 0);
out:
	mac_vnode_label_free(intlabel);
	return (error);
}

int
sys_mac_syscall(struct thread *td, struct mac_syscall_args *uap)
{
	struct mac_policy_conf *mpc;
	char target[MAC_MAX_POLICY_NAME];
	int error;

	error = copyinstr(uap->policy, target, sizeof(target), NULL);
	if (error)
		return (error);

	error = ENOSYS;
	LIST_FOREACH(mpc, &mac_static_policy_list, mpc_list) {
		if (strcmp(mpc->mpc_name, target) == 0 &&
		    mpc->mpc_ops->mpo_syscall != NULL) {
			error = mpc->mpc_ops->mpo_syscall(td,
			    uap->call, uap->arg);
			goto out;
		}
	}

	if (!LIST_EMPTY(&mac_policy_list)) {
		mac_policy_slock_sleep();
		LIST_FOREACH(mpc, &mac_policy_list, mpc_list) {
			if (strcmp(mpc->mpc_name, target) == 0 &&
			    mpc->mpc_ops->mpo_syscall != NULL) {
				error = mpc->mpc_ops->mpo_syscall(td,
				    uap->call, uap->arg);
				break;
			}
		}
		mac_policy_sunlock_sleep();
	}
out:
	return (error);
}

#else /* !MAC */

int
sys___mac_get_pid(struct thread *td, struct __mac_get_pid_args *uap)
{

	return (ENOSYS);
}

int
sys___mac_get_proc(struct thread *td, struct __mac_get_proc_args *uap)
{

	return (ENOSYS);
}

int
sys___mac_set_proc(struct thread *td, struct __mac_set_proc_args *uap)
{

	return (ENOSYS);
}

int
sys___mac_get_fd(struct thread *td, struct __mac_get_fd_args *uap)
{

	return (ENOSYS);
}

int
sys___mac_get_file(struct thread *td, struct __mac_get_file_args *uap)
{

	return (ENOSYS);
}

int
sys___mac_get_link(struct thread *td, struct __mac_get_link_args *uap)
{

	return (ENOSYS);
}

int
sys___mac_set_fd(struct thread *td, struct __mac_set_fd_args *uap)
{

	return (ENOSYS);
}

int
sys___mac_set_file(struct thread *td, struct __mac_set_file_args *uap)
{

	return (ENOSYS);
}

int
sys___mac_set_link(struct thread *td, struct __mac_set_link_args *uap)
{

	return (ENOSYS);
}

int
sys_mac_syscall(struct thread *td, struct mac_syscall_args *uap)
{

	return (ENOSYS);
}

#endif /* !MAC */