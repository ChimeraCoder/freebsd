
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

/*
 * STATIC VARIABLES:
 */
static nfsfattr rootfattr = {NFDIR, 0040555, 2, 0, 0, 512, 512, 0,
			     1, 0, ROOTID};
static nfsfattr slinkfattr = {NFLNK, 0120777, 1, 0, 0, NFS_MAXPATHLEN, 512, 0,
			      (NFS_MAXPATHLEN + 1) / 512, 0, SLINKID};
				/* user name file attributes */
static nfsfattr un_fattr = {NFLNK, 0120777, 1, 0, 0, NFS_MAXPATHLEN, 512, 0,
			    (NFS_MAXPATHLEN + 1) / 512, 0, INVALIDID};
static int started;
static am_nfs_fh slink;
static am_nfs_fh un_fhandle;

/*
 * GLOBALS:
 */
am_nfs_fh root;
am_nfs_fh *root_fhp =		&root;


/* initialize NFS file handles for hlfsd */
void
hlfsd_init_filehandles(void)
{
  u_int ui;

  ui = ROOTID;
  memcpy(root.fh_data, &ui, sizeof(ui));

  ui = SLINKID;
  memcpy(slink.fh_data, &ui, sizeof(ui));

  ui = INVALIDID;
  memcpy(un_fhandle.fh_data, &ui, sizeof(ui));
}


voidp
nfsproc_null_2_svc(voidp argp, struct svc_req *rqstp)
{
  static char res;

  return (voidp) &res;
}


/* compare if two filehandles are equal */
static int
eq_fh(const am_nfs_fh *fh1, const am_nfs_fh *fh2)
{
  return (!memcmp((char *) fh1, (char *) fh2, sizeof(am_nfs_fh)));
}


nfsattrstat *
nfsproc_getattr_2_svc(am_nfs_fh *argp, struct svc_req *rqstp)
{
  static nfsattrstat res;
  uid_t uid = (uid_t) INVALIDID;
  gid_t gid = (gid_t) INVALIDID;

  if (!started) {
    started++;
    rootfattr.na_ctime = startup;
    rootfattr.na_mtime = startup;
    slinkfattr.na_ctime = startup;
    slinkfattr.na_mtime = startup;
    un_fattr.na_ctime = startup;
    un_fattr.na_mtime = startup;
  }

  if (getcreds(rqstp, &uid, &gid, nfsxprt) < 0) {
    res.ns_status = NFSERR_STALE;
    return &res;
  }
  if (eq_fh(argp, &root)) {
#if 0
    /*
     * XXX: increment mtime of parent directory, causes NFS clients to
     * invalidate their cache for that directory.
     * Some NFS clients may need this code.
     */
    if (uid != rootfattr.na_uid) {
      clocktime(&rootfattr.na_mtime);
      rootfattr.na_uid = uid;
    }
#endif
    res.ns_status = NFS_OK;
    res.ns_u.ns_attr_u = rootfattr;
  } else if (eq_fh(argp, &slink)) {

#ifndef MNT2_NFS_OPT_SYMTTL
    /*
     * This code is needed to defeat Solaris 2.4's (and newer) symlink
     * values cache.  It forces the last-modified time of the symlink to be
     * current.  It is not needed if the O/S has an nfs flag to turn off the
     * symlink-cache at mount time (such as Irix 5.x and 6.x). -Erez.
     *
     * Additionally, Linux currently ignores the nt_useconds field,
     * so we must update the nt_seconds field every time.
     */
    if (uid != slinkfattr.na_uid) {
      clocktime(&slinkfattr.na_mtime);
      slinkfattr.na_uid = uid;
    }
#endif /* not MNT2_NFS_OPT_SYMTTL */

    res.ns_status = NFS_OK;
    res.ns_u.ns_attr_u = slinkfattr;
  } else {
    if (gid != hlfs_gid) {
      res.ns_status = NFSERR_STALE;
    } else {
      memset((char *) &uid, 0, sizeof(int));
      uid = *(u_int *) argp->fh_data;
      if (plt_search(uid) != (uid2home_t *) NULL) {
	res.ns_status = NFS_OK;
	un_fattr.na_fileid = uid;
	res.ns_u.ns_attr_u = un_fattr;
	dlog("nfs_getattr: successful search for uid=%ld, gid=%ld",
	     (long) uid, (long) gid);
      } else {			/* not found */
	res.ns_status = NFSERR_STALE;
      }
    }
  }
  return &res;
}


nfsattrstat *
nfsproc_setattr_2_svc(nfssattrargs *argp, struct svc_req *rqstp)
{
  static nfsattrstat res = {NFSERR_ROFS};

  return &res;
}


voidp
nfsproc_root_2_svc(voidp argp, struct svc_req *rqstp)
{
  static char res;

  return (voidp) &res;
}


nfsdiropres *
nfsproc_lookup_2_svc(nfsdiropargs *argp, struct svc_req *rqstp)
{
  static nfsdiropres res;
  int idx;
  uid_t uid = (uid_t) INVALIDID;
  gid_t gid = (gid_t) INVALIDID;

  if (!started) {
    started++;
    rootfattr.na_ctime = startup;
    rootfattr.na_mtime = startup;
    slinkfattr.na_ctime = startup;
    slinkfattr.na_mtime = startup;
    un_fattr.na_ctime = startup;
    un_fattr.na_mtime = startup;
  }

  if (eq_fh(&argp->da_fhandle, &slink)) {
    res.dr_status = NFSERR_NOTDIR;
    return &res;
  }

  if (getcreds(rqstp, &uid, &gid, nfsxprt) < 0) {
    res.dr_status = NFSERR_NOENT;
    return &res;
  }
  if (eq_fh(&argp->da_fhandle, &root)) {
    if (argp->da_name[0] == '.' &&
	(argp->da_name[1] == '\0' ||
	 (argp->da_name[1] == '.' &&
	  argp->da_name[2] == '\0'))) {
#if 0
    /*
     * XXX: increment mtime of parent directory, causes NFS clients to
     * invalidate their cache for that directory.
     * Some NFS clients may need this code.
     */
      if (uid != rootfattr.na_uid) {
	clocktime(&rootfattr.na_mtime);
	rootfattr.na_uid = uid;
      }
#endif
      res.dr_u.dr_drok_u.drok_fhandle = root;
      res.dr_u.dr_drok_u.drok_attributes = rootfattr;
      res.dr_status = NFS_OK;
      return &res;
    }

    if (STREQ(argp->da_name, slinkname)) {
#ifndef MNT2_NFS_OPT_SYMTTL
      /*
       * This code is needed to defeat Solaris 2.4's (and newer) symlink
       * values cache.  It forces the last-modified time of the symlink to be
       * current.  It is not needed if the O/S has an nfs flag to turn off the
       * symlink-cache at mount time (such as Irix 5.x and 6.x). -Erez.
       *
       * Additionally, Linux currently ignores the nt_useconds field,
       * so we must update the nt_seconds field every time.
       */
      if (uid != slinkfattr.na_uid) {
	clocktime(&slinkfattr.na_mtime);
	slinkfattr.na_uid = uid;
      }
#endif /* not MNT2_NFS_OPT_SYMTTL */
      res.dr_u.dr_drok_u.drok_fhandle = slink;
      res.dr_u.dr_drok_u.drok_attributes = slinkfattr;
      res.dr_status = NFS_OK;
      return &res;
    }

    if (gid != hlfs_gid) {
      res.dr_status = NFSERR_NOENT;
      return &res;
    }

    /* if gets here, gid == hlfs_gid */
    if ((idx = untab_index(argp->da_name)) < 0) {
      res.dr_status = NFSERR_NOENT;
      return &res;
    } else {			/* entry found and gid is permitted */
      un_fattr.na_fileid = untab[idx].uid;
      res.dr_u.dr_drok_u.drok_attributes = un_fattr;
      memset((char *) &un_fhandle, 0, sizeof(am_nfs_fh));
      *(u_int *) un_fhandle.fh_data = (u_int) untab[idx].uid;
      xstrlcpy((char *) &un_fhandle.fh_data[sizeof(int)],
	       untab[idx].username,
	       sizeof(am_nfs_fh) - sizeof(int));
      res.dr_u.dr_drok_u.drok_fhandle = un_fhandle;
      res.dr_status = NFS_OK;
      dlog("nfs_lookup: successful lookup for uid=%ld, gid=%ld: username=%s",
	   (long) uid, (long) gid, untab[idx].username);
      return &res;
    }
  } /* end of "if (eq_fh(argp->dir.data, root.data)) {" */

  res.dr_status = NFSERR_STALE;
  return &res;
}


nfsreadlinkres *
nfsproc_readlink_2_svc(am_nfs_fh *argp, struct svc_req *rqstp)
{
  static nfsreadlinkres res;
  uid_t userid = (uid_t) INVALIDID;
  gid_t groupid = hlfs_gid + 1;	/* anything not hlfs_gid */
  int retval = 0;
  char *path_val = (char *) NULL;
  char *username;
  static uid_t last_uid = (uid_t) INVALIDID;

  if (eq_fh(argp, &root)) {
    res.rlr_status = NFSERR_ISDIR;
  } else if (eq_fh(argp, &slink)) {
    if (getcreds(rqstp, &userid, &groupid, nfsxprt) < 0)
      return (nfsreadlinkres *) NULL;

    clocktime(&slinkfattr.na_atime);

    res.rlr_status = NFS_OK;
    if (groupid == hlfs_gid) {
      res.rlr_u.rlr_data_u = DOTSTRING;
    } else if (!(res.rlr_u.rlr_data_u = path_val = homedir(userid, groupid))) {
      /*
       * parent process (fork in homedir()) continues
       * processing, by getting a NULL returned as a
       * "special".  Child returns result.
       */
      return (nfsreadlinkres *) NULL;
    }

  } else {			/* check if asked for user mailbox */

    if (getcreds(rqstp, &userid, &groupid, nfsxprt) < 0) {
      return (nfsreadlinkres *) NULL;
    }

    if (groupid == hlfs_gid) {
      memset((char *) &userid, 0, sizeof(int));
      userid = *(u_int *) argp->fh_data;
      username = (char *) &argp->fh_data[sizeof(int)];
      if (!(res.rlr_u.rlr_data_u = mailbox(userid, username)))
	return (nfsreadlinkres *) NULL;
    } else {
      res.rlr_status = NFSERR_STALE;
    }
  }

  /* print info, but try to avoid repetitions */
  if (userid != last_uid) {
    plog(XLOG_USER, "mailbox for uid=%ld, gid=%ld is %s",
	 (long) userid, (long) groupid, (char *) res.rlr_u.rlr_data_u);
    last_uid = userid;
  }

  /* I don't think it will pass this if -D fork */
  if (serverpid == getpid())
    return &res;

  if (!svc_sendreply(nfsxprt, (XDRPROC_T_TYPE) xdr_readlinkres, (SVC_IN_ARG_TYPE) &res))
    svcerr_systemerr(nfsxprt);

  /*
   * Child exists here.   We need to determine which
   * exist status to return.  The exit status
   * is gathered using wait() and determines
   * if we returned $HOME/.hlfsspool or $ALTDIR.  The parent
   * needs this info so it can update the lookup table.
   */
  if (path_val && alt_spooldir && STREQ(path_val, alt_spooldir))
    retval = 1;		/* could not get real home dir (or uid 0 user) */
  else
    retval = 0;

  /*
   * If asked for -D fork, then must return the value,
   * NOT exit, or else the main hlfsd server exits.
   * Bug: where is that status information being collected?
   */
  if (amuDebug(D_FORK))
    return &res;

  exit(retval);
}


nfsreadres *
nfsproc_read_2_svc(nfsreadargs *argp, struct svc_req *rqstp)
{
  static nfsreadres res = {NFSERR_ACCES};

  return &res;
}


voidp
nfsproc_writecache_2_svc(voidp argp, struct svc_req *rqstp)
{
  static char res;

  return (voidp) &res;
}


nfsattrstat *
nfsproc_write_2_svc(nfswriteargs *argp, struct svc_req *rqstp)
{
  static nfsattrstat res = {NFSERR_ROFS};

  return &res;
}


nfsdiropres *
nfsproc_create_2_svc(nfscreateargs *argp, struct svc_req *rqstp)
{
  static nfsdiropres res = {NFSERR_ROFS};

  return &res;
}


nfsstat *
nfsproc_remove_2_svc(nfsdiropargs *argp, struct svc_req *rqstp)
{
  static nfsstat res = {NFSERR_ROFS};

  return &res;
}


nfsstat *
nfsproc_rename_2_svc(nfsrenameargs *argp, struct svc_req *rqstp)
{
  static nfsstat res = {NFSERR_ROFS};

  return &res;
}


nfsstat *
nfsproc_link_2_svc(nfslinkargs *argp, struct svc_req *rqstp)
{
  static nfsstat res = {NFSERR_ROFS};

  return &res;
}


nfsstat *
nfsproc_symlink_2_svc(nfssymlinkargs *argp, struct svc_req *rqstp)
{
  static nfsstat res = {NFSERR_ROFS};

  return &res;
}


nfsdiropres *
nfsproc_mkdir_2_svc(nfscreateargs *argp, struct svc_req *rqstp)
{
  static nfsdiropres res = {NFSERR_ROFS};

  return &res;
}


nfsstat *
nfsproc_rmdir_2_svc(nfsdiropargs *argp, struct svc_req *rqstp)
{
  static nfsstat res = {NFSERR_ROFS};

  return &res;
}


nfsreaddirres *
nfsproc_readdir_2_svc(nfsreaddirargs *argp, struct svc_req *rqstp)
{
  static nfsreaddirres res;
  static nfsentry slinkent = {SLINKID, 0, {SLINKCOOKIE}};
  static nfsentry dotdotent = {ROOTID, "..", {DOTDOTCOOKIE}, &slinkent};
  static nfsentry dotent = {ROOTID, ".", {DOTCOOKIE}, &dotdotent};

  slinkent.ne_name = slinkname;

  if (eq_fh(&argp->rda_fhandle, &slink)) {
    res.rdr_status = NFSERR_NOTDIR;
  } else if (eq_fh(&argp->rda_fhandle, &root)) {
    clocktime(&rootfattr.na_atime);

    res.rdr_status = NFS_OK;
    switch (argp->rda_cookie[0]) {
    case 0:
      res.rdr_u.rdr_reply_u.dl_entries = &dotent;
      break;
    case DOTCOOKIE:
      res.rdr_u.rdr_reply_u.dl_entries = &dotdotent;
      break;
    case DOTDOTCOOKIE:
      res.rdr_u.rdr_reply_u.dl_entries = &slinkent;
      break;
    case SLINKCOOKIE:
      res.rdr_u.rdr_reply_u.dl_entries = (nfsentry *) 0;
      break;
    }
    res.rdr_u.rdr_reply_u.dl_eof = TRUE;
  } else {
    res.rdr_status = NFSERR_STALE;
  }
  return &res;
}


nfsstatfsres *
nfsproc_statfs_2_svc(am_nfs_fh *argp, struct svc_req *rqstp)
{
  static nfsstatfsres res = {NFS_OK};

  res.sfr_u.sfr_reply_u.sfrok_tsize = 1024;
  res.sfr_u.sfr_reply_u.sfrok_bsize = 1024;

  /*
   * Some "df" programs automatically assume that file systems
   * with zero blocks are meta-filesystems served by automounters.
   */
  res.sfr_u.sfr_reply_u.sfrok_blocks = 0;
  res.sfr_u.sfr_reply_u.sfrok_bfree = 0;
  res.sfr_u.sfr_reply_u.sfrok_bavail = 0;

  return &res;
}