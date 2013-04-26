
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <hlfsd.h>

/* EXTERNAL FUNCTIONS */
extern voidp nfsproc_null_2_svc(voidp, struct svc_req *);
extern nfsattrstat *nfsproc_getattr_2_svc(am_nfs_fh *, struct svc_req *);
extern nfsattrstat *nfsproc_setattr_2_svc(nfssattrargs *, struct svc_req *);
extern voidp nfsproc_root_2_svc(voidp, struct svc_req *);
extern nfsdiropres *nfsproc_lookup_2_svc(nfsdiropargs *, struct svc_req *);
extern nfsreadlinkres *nfsproc_readlink_2_svc(am_nfs_fh *, struct svc_req *);
extern nfsreadres *nfsproc_read_2_svc(nfsreadargs *, struct svc_req *);
extern voidp nfsproc_writecache_2_svc(voidp, struct svc_req *);
extern nfsattrstat *nfsproc_write_2_svc(nfswriteargs *, struct svc_req *);
extern nfsdiropres *nfsproc_create_2_svc(nfscreateargs *, struct svc_req *);
extern nfsstat *nfsproc_remove_2_svc(nfsdiropargs *, struct svc_req *);
extern nfsstat *nfsproc_rename_2_svc(nfsrenameargs *, struct svc_req *);
extern nfsstat *nfsproc_link_2_svc(nfslinkargs *, struct svc_req *);
extern nfsstat *nfsproc_symlink_2_svc(nfssymlinkargs *, struct svc_req *);
extern nfsdiropres *nfsproc_mkdir_2_svc(nfscreateargs *, struct svc_req *);
extern nfsstat *nfsproc_rmdir_2_svc(nfsdiropargs *, struct svc_req *);
extern nfsreaddirres *nfsproc_readdir_2_svc(nfsreaddirargs *, struct svc_req *);
extern nfsstatfsres *nfsproc_statfs_2_svc(am_nfs_fh *, struct svc_req *);

/* GLOBALS */
SVCXPRT *nfs_program_2_transp;

/* TYPEDEFS */
typedef char *(*nfssvcproc_t)(voidp, struct svc_req *);


void
nfs_program_2(struct svc_req *rqstp, SVCXPRT *transp)
{
  union {
    am_nfs_fh		nfsproc_getattr_2_arg;
    nfssattrargs	nfsproc_setattr_2_arg;
    nfsdiropargs	nfsproc_lookup_2_arg;
    am_nfs_fh		nfsproc_readlink_2_arg;
    nfsreadargs		nfsproc_read_2_arg;
    nfswriteargs	nfsproc_write_2_arg;
    nfscreateargs	nfsproc_create_2_arg;
    nfsdiropargs	nfsproc_remove_2_arg;
    nfsrenameargs	nfsproc_rename_2_arg;
    nfslinkargs		nfsproc_link_2_arg;
    nfssymlinkargs	nfsproc_symlink_2_arg;
    nfscreateargs	nfsproc_mkdir_2_arg;
    nfsdiropargs	nfsproc_rmdir_2_arg;
    nfsreaddirargs	nfsproc_readdir_2_arg;
    am_nfs_fh		nfsproc_statfs_2_arg;
  } argument;
  char *result;
  xdrproc_t xdr_argument, xdr_result;
  nfssvcproc_t local;

  nfs_program_2_transp = NULL;

  switch (rqstp->rq_proc) {

  case NFSPROC_NULL:
    xdr_argument = (xdrproc_t) xdr_void;
    xdr_result = (xdrproc_t) xdr_void;
    local = (nfssvcproc_t) nfsproc_null_2_svc;
    break;

  case NFSPROC_GETATTR:
    xdr_argument = (xdrproc_t) xdr_nfs_fh;
    xdr_result = (xdrproc_t) xdr_attrstat;
    local = (nfssvcproc_t) nfsproc_getattr_2_svc;
    break;

  case NFSPROC_SETATTR:
    xdr_argument = (xdrproc_t) xdr_sattrargs;
    xdr_result = (xdrproc_t) xdr_attrstat;
    local = (nfssvcproc_t) nfsproc_setattr_2_svc;
    break;

  case NFSPROC_ROOT:
    xdr_argument = (xdrproc_t) xdr_void;
    xdr_result = (xdrproc_t) xdr_void;
    local = (nfssvcproc_t) nfsproc_root_2_svc;
    break;

  case NFSPROC_LOOKUP:
    xdr_argument = (xdrproc_t) xdr_diropargs;
    xdr_result = (xdrproc_t) xdr_diropres;
    local = (nfssvcproc_t) nfsproc_lookup_2_svc;
    /*
     * Cheap way to pass transp down to afs_lookuppn so it can
     * be stored in the am_node structure and later used for
     * quick_reply().
     */
    nfs_program_2_transp = transp;
    break;

  case NFSPROC_READLINK:
    xdr_argument = (xdrproc_t) xdr_nfs_fh;
    xdr_result = (xdrproc_t) xdr_readlinkres;
    local = (nfssvcproc_t) nfsproc_readlink_2_svc;
    break;

  case NFSPROC_READ:
    xdr_argument = (xdrproc_t) xdr_readargs;
    xdr_result = (xdrproc_t) xdr_readres;
    local = (nfssvcproc_t) nfsproc_read_2_svc;
    break;

  case NFSPROC_WRITECACHE:
    xdr_argument = (xdrproc_t) xdr_void;
    xdr_result = (xdrproc_t) xdr_void;
    local = (nfssvcproc_t) nfsproc_writecache_2_svc;
    break;

  case NFSPROC_WRITE:
    xdr_argument = (xdrproc_t) xdr_writeargs;
    xdr_result = (xdrproc_t) xdr_attrstat;
    local = (nfssvcproc_t) nfsproc_write_2_svc;
    break;

  case NFSPROC_CREATE:
    xdr_argument = (xdrproc_t) xdr_createargs;
    xdr_result = (xdrproc_t) xdr_diropres;
    local = (nfssvcproc_t) nfsproc_create_2_svc;
    break;

  case NFSPROC_REMOVE:
    xdr_argument = (xdrproc_t) xdr_diropargs;
    xdr_result = (xdrproc_t) xdr_nfsstat;
    local = (nfssvcproc_t) nfsproc_remove_2_svc;
    break;

  case NFSPROC_RENAME:
    xdr_argument = (xdrproc_t) xdr_renameargs;
    xdr_result = (xdrproc_t) xdr_nfsstat;
    local = (nfssvcproc_t) nfsproc_rename_2_svc;
    break;

  case NFSPROC_LINK:
    xdr_argument = (xdrproc_t) xdr_linkargs;
    xdr_result = (xdrproc_t) xdr_nfsstat;
    local = (nfssvcproc_t) nfsproc_link_2_svc;
    break;

  case NFSPROC_SYMLINK:
    xdr_argument = (xdrproc_t) xdr_symlinkargs;
    xdr_result = (xdrproc_t) xdr_nfsstat;
    local = (nfssvcproc_t) nfsproc_symlink_2_svc;
    break;

  case NFSPROC_MKDIR:
    xdr_argument = (xdrproc_t) xdr_createargs;
    xdr_result = (xdrproc_t) xdr_diropres;
    local = (nfssvcproc_t) nfsproc_mkdir_2_svc;
    break;

  case NFSPROC_RMDIR:
    xdr_argument = (xdrproc_t) xdr_diropargs;
    xdr_result = (xdrproc_t) xdr_nfsstat;
    local = (nfssvcproc_t) nfsproc_rmdir_2_svc;
    break;

  case NFSPROC_READDIR:
    xdr_argument = (xdrproc_t) xdr_readdirargs;
    xdr_result = (xdrproc_t) xdr_readdirres;
    local = (nfssvcproc_t) nfsproc_readdir_2_svc;
    break;

  case NFSPROC_STATFS:
    xdr_argument = (xdrproc_t) xdr_nfs_fh;
    xdr_result = (xdrproc_t) xdr_statfsres;
    local = (nfssvcproc_t) nfsproc_statfs_2_svc;
    break;

  default:
    svcerr_noproc(transp);
    return;
  }

  memset((char *) &argument, 0, sizeof(argument));
  if (!svc_getargs(transp,
		   (XDRPROC_T_TYPE) xdr_argument,
		   (SVC_IN_ARG_TYPE) &argument)) {
    plog(XLOG_ERROR,
	 "NFS xdr decode failed for %d %d %d",
	 (int) rqstp->rq_prog, (int) rqstp->rq_vers, (int) rqstp->rq_proc);
    svcerr_decode(transp);
    return;
  }
  result = (*local) (&argument, rqstp);

  nfs_program_2_transp = NULL;

  if (result != NULL && !svc_sendreply(transp,
				       (XDRPROC_T_TYPE) xdr_result,
				       result)) {
    svcerr_systemerr(transp);
  }
  if (!svc_freeargs(transp,
		    (XDRPROC_T_TYPE) xdr_argument,
		    (SVC_IN_ARG_TYPE) & argument)) {
    plog(XLOG_FATAL, "unable to free rpc arguments in nfs_program_2");
    going_down(1);
  }
}