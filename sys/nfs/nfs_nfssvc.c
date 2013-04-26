
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

#include "opt_nfs.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/sysproto.h>
#include <sys/kernel.h>
#include <sys/sysctl.h>
#include <sys/priv.h>
#include <sys/proc.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/syscall.h>
#include <sys/sysproto.h>

#include <security/audit/audit.h>

#include <nfs/nfssvc.h>

static int nfssvc_offset = SYS_nfssvc;
static struct sysent nfssvc_prev_sysent;
MAKE_SYSENT(nfssvc);

/*
 * This tiny module simply handles the nfssvc() system call. The other
 * nfs modules that use the system call register themselves by setting
 * the nfsd_call_xxx function pointers non-NULL.
 */

int (*nfsd_call_nfsserver)(struct thread *, struct nfssvc_args *) = NULL;
int (*nfsd_call_nfscommon)(struct thread *, struct nfssvc_args *) = NULL;
int (*nfsd_call_nfscl)(struct thread *, struct nfssvc_args *) = NULL;
int (*nfsd_call_nfsd)(struct thread *, struct nfssvc_args *) = NULL;

/*
 * Nfs server psuedo system call for the nfsd's
 */
int
sys_nfssvc(struct thread *td, struct nfssvc_args *uap)
{
	int error;

	KASSERT(!mtx_owned(&Giant), ("nfssvc(): called with Giant"));

	AUDIT_ARG_CMD(uap->flag);

	/* Allow anyone to get the stats. */
	if ((uap->flag & ~NFSSVC_GETSTATS) != 0) {
		error = priv_check(td, PRIV_NFS_DAEMON);
		if (error != 0)
			return (error);
	}
	error = EINVAL;
	if ((uap->flag & (NFSSVC_ADDSOCK | NFSSVC_OLDNFSD | NFSSVC_NFSD)) &&
	    nfsd_call_nfsserver != NULL)
		error = (*nfsd_call_nfsserver)(td, uap);
	else if ((uap->flag & (NFSSVC_CBADDSOCK | NFSSVC_NFSCBD |
	    NFSSVC_DUMPMNTOPTS)) && nfsd_call_nfscl != NULL)
		error = (*nfsd_call_nfscl)(td, uap);
	else if ((uap->flag & (NFSSVC_IDNAME | NFSSVC_GETSTATS |
	    NFSSVC_GSSDADDPORT | NFSSVC_GSSDADDFIRST | NFSSVC_GSSDDELETEALL |
	    NFSSVC_NFSUSERDPORT | NFSSVC_NFSUSERDDELPORT)) &&
	    nfsd_call_nfscommon != NULL)
		error = (*nfsd_call_nfscommon)(td, uap);
	else if ((uap->flag & (NFSSVC_NFSDNFSD | NFSSVC_NFSDADDSOCK |
	    NFSSVC_PUBLICFH | NFSSVC_V4ROOTEXPORT | NFSSVC_NOPUBLICFH |
	    NFSSVC_STABLERESTART | NFSSVC_ADMINREVOKE |
	    NFSSVC_DUMPCLIENTS | NFSSVC_DUMPLOCKS | NFSSVC_BACKUPSTABLE |
	    NFSSVC_SUSPENDNFSD | NFSSVC_RESUMENFSD)) &&
	    nfsd_call_nfsd != NULL)
		error = (*nfsd_call_nfsd)(td, uap);
	if (error == EINTR || error == ERESTART)
		error = 0;
	return (error);
}

/*
 * Called once to initialize data structures...
 */
static int
nfssvc_modevent(module_t mod, int type, void *data)
{
	static int registered;
	int error = 0;

	switch (type) {
	case MOD_LOAD:
		error = syscall_register(&nfssvc_offset, &nfssvc_sysent,
		    &nfssvc_prev_sysent);
		if (error)
			break;
		registered = 1;
		break;

	case MOD_UNLOAD:
		if (nfsd_call_nfsserver != NULL || nfsd_call_nfscommon != NULL
		    || nfsd_call_nfscl != NULL || nfsd_call_nfsd != NULL) {
			error = EBUSY;
			break;
		}
		if (registered)
			syscall_deregister(&nfssvc_offset, &nfssvc_prev_sysent);
		registered = 0;
		break;
	default:
		error = EOPNOTSUPP;
		break;
	}
	return error;
}
static moduledata_t nfssvc_mod = {
	"nfssvc",
	nfssvc_modevent,
	NULL,
};
DECLARE_MODULE(nfssvc, nfssvc_mod, SI_SUB_VFS, SI_ORDER_ANY);

/* So that loader and kldload(2) can find us, wherever we are.. */
MODULE_VERSION(nfssvc, 1);