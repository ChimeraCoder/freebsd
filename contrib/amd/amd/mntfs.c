
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
#include <amd.h>

qelem mfhead = {&mfhead, &mfhead};

int mntfs_allocated;


mntfs *
dup_mntfs(mntfs *mf)
{
  if (mf->mf_refc == 0) {
    if (mf->mf_cid)
      untimeout(mf->mf_cid);
    mf->mf_cid = 0;
  }
  mf->mf_refc++;

  return mf;
}


static void
init_mntfs(mntfs *mf, am_ops *ops, am_opts *mo, char *mp, char *info, char *auto_opts, char *mopts, char *remopts)
{
  mf->mf_ops = ops;
  mf->mf_fsflags = ops->nfs_fs_flags;
  mf->mf_fo = mo;
  mf->mf_mount = strdup(mp);
  mf->mf_info = strdup(info);
  mf->mf_auto = strdup(auto_opts);
  mf->mf_mopts = strdup(mopts);
  mf->mf_remopts = strdup(remopts);
  mf->mf_loopdev = NULL;
  mf->mf_refc = 1;
  mf->mf_flags = 0;
  mf->mf_error = -1;
  mf->mf_cid = 0;
  mf->mf_private = 0;
  mf->mf_prfree = 0;

  if (ops->ffserver)
    mf->mf_server = (*ops->ffserver) (mf);
  else
    mf->mf_server = 0;
}


static mntfs *
alloc_mntfs(am_ops *ops, am_opts *mo, char *mp, char *info, char *auto_opts, char *mopts, char *remopts)
{
  mntfs *mf = ALLOC(struct mntfs);

  init_mntfs(mf, ops, mo, mp, info, auto_opts, mopts, remopts);
  ins_que(&mf->mf_q, &mfhead);
  mntfs_allocated++;

  return mf;
}


/* find a matching mntfs in our list */
mntfs *
locate_mntfs(am_ops *ops, am_opts *mo, char *mp, char *info, char *auto_opts, char *mopts, char *remopts)
{
  mntfs *mf;

  dlog("Locating mntfs reference to (%s,%s)", mp, info);

  ITER(mf, mntfs, &mfhead) {
    /*
     * For backwards compatibility purposes, we treat already-mounted
     * filesystems differently and only require a match of their mount point,
     * not of their server info. After all, there is little we can do if
     * the user asks us to mount two different things onto the same mount: one
     * will always cover the other one.
     */
    if (STREQ(mf->mf_mount, mp) &&
	((mf->mf_flags & MFF_MOUNTED && !(mf->mf_fsflags & FS_DIRECT))
	 || (STREQ(mf->mf_info, info) && mf->mf_ops == ops))) {
      /*
       * Handle cases where error ops are involved
       */
      if (ops == &amfs_error_ops) {
	/*
	 * If the existing ops are not amfs_error_ops
	 * then continue...
	 */
	if (mf->mf_ops != &amfs_error_ops)
	  continue;
	return dup_mntfs(mf);
      }

      dlog("mf->mf_flags = %#x", mf->mf_flags);
      mf->mf_fo = mo;
      if ((mf->mf_flags & MFF_RESTART) && amd_state < Finishing) {
	/*
	 * Restart a previously mounted filesystem.
	 */
	dlog("Restarting filesystem %s", mf->mf_mount);

	/*
	 * If we are restarting an amd internal filesystem,
	 * we need to initialize it a bit.
	 *
	 * We know it's internal because it is marked as toplvl.
	 */
	if (mf->mf_ops == &amfs_toplvl_ops) {
	  mf->mf_ops = ops;
	  mf->mf_info = strealloc(mf->mf_info, info);
	  ops->mounted(mf);	/* XXX: not right, but will do for now */
	}

	return mf;
      }

      if (!(mf->mf_flags & (MFF_MOUNTED | MFF_MOUNTING | MFF_UNMOUNTING))) {
	fserver *fs;
	mf->mf_flags &= ~MFF_ERROR;
	mf->mf_error = -1;
	mf->mf_auto = strealloc(mf->mf_auto, auto_opts);
	mf->mf_mopts = strealloc(mf->mf_mopts, mopts);
	mf->mf_remopts = strealloc(mf->mf_remopts, remopts);
	mf->mf_info = strealloc(mf->mf_info, info);

	if (mf->mf_private && mf->mf_prfree) {
	  mf->mf_prfree(mf->mf_private);
	  mf->mf_private = 0;
	}

	fs = ops->ffserver ? (*ops->ffserver) (mf) : (fserver *) NULL;
	if (mf->mf_server)
	  free_srvr(mf->mf_server);
	mf->mf_server = fs;
      }
      return dup_mntfs(mf);
    } /* end of "if (STREQ(mf-> ..." */
  } /* end of ITER */

  return 0;
}


/* find a matching mntfs in our list, create a new one if none is found */
mntfs *
find_mntfs(am_ops *ops, am_opts *mo, char *mp, char *info, char *auto_opts, char *mopts, char *remopts)
{
  mntfs *mf = locate_mntfs(ops, mo, mp, info, auto_opts, mopts, remopts);
  if (mf)
    return mf;

  return alloc_mntfs(ops, mo, mp, info, auto_opts, mopts, remopts);
}


mntfs *
new_mntfs(void)
{
  return alloc_mntfs(&amfs_error_ops, (am_opts *) 0, "//nil//", ".", "", "", "");
}


static void
uninit_mntfs(mntfs *mf)
{
  if (mf->mf_auto)
    XFREE(mf->mf_auto);
  if (mf->mf_mopts)
    XFREE(mf->mf_mopts);
  if (mf->mf_remopts)
    XFREE(mf->mf_remopts);
  if (mf->mf_info)
    XFREE(mf->mf_info);
  if (mf->mf_private && mf->mf_prfree)
    (*mf->mf_prfree) (mf->mf_private);

  if (mf->mf_mount)
    XFREE(mf->mf_mount);

  /*
   * Clean up the file server
   */
  if (mf->mf_server)
    free_srvr(mf->mf_server);

  /*
   * Don't do a callback on this mount
   */
  if (mf->mf_cid) {
    untimeout(mf->mf_cid);
    mf->mf_cid = 0;
  }
}


static void
discard_mntfs(voidp v)
{
  mntfs *mf = v;

  rem_que(&mf->mf_q);

  /*
   * Free memory
   */
  uninit_mntfs(mf);
  XFREE(mf);

  --mntfs_allocated;
}


void
flush_mntfs(void)
{
  mntfs *mf;

  mf = AM_FIRST(mntfs, &mfhead);
  while (mf != HEAD(mntfs, &mfhead)) {
    mntfs *mf2 = mf;
    mf = NEXT(mntfs, mf);
    if (mf2->mf_refc == 0 && mf2->mf_cid)
      discard_mntfs(mf2);
  }
}


void
free_mntfs(opaque_t arg)
{
  mntfs *mf = (mntfs *) arg;

  dlog("free_mntfs <%s> type %s mf_refc %d flags %x",
       mf->mf_mount, mf->mf_ops->fs_type, mf->mf_refc, mf->mf_flags);

  /*
   * We shouldn't ever be called to free something that has
   * a non-positive refcount.  Something is badly wrong if
   * we have been!  Ignore the request for now...
   */
  if (mf->mf_refc <= 0) {
    plog(XLOG_ERROR, "IGNORING free_mntfs for <%s>: refc %d, flags %x (bug?)",
         mf->mf_mount, mf->mf_refc, mf->mf_flags);
    return;
  }

  /* don't discard last reference of a restarted/kept mntfs */
  if (mf->mf_refc == 1 && mf->mf_flags & MFF_RSTKEEP) {
    plog(XLOG_ERROR, "IGNORING free_mntfs for <%s>: refc %d, flags %x (restarted)",
         mf->mf_mount, mf->mf_refc, mf->mf_flags);
    return;
  }

  if (--mf->mf_refc == 0) {
    if (mf->mf_flags & MFF_MOUNTED) {
      int quoted;
      mf->mf_flags &= ~MFF_MOUNTED;

      /*
       * Record for posterity
       */
      quoted = strchr(mf->mf_info, ' ') != 0;	/* cheap */
      plog(XLOG_INFO, "%s%s%s %sed fstype %s from %s",
	   quoted ? "\"" : "",
	   mf->mf_info,
	   quoted ? "\"" : "",
	   mf->mf_error ? "discard" : "unmount",
	   mf->mf_ops->fs_type, mf->mf_mount);
    }

    if (mf->mf_fsflags & FS_DISCARD) {
      dlog("Immediately discarding mntfs for %s", mf->mf_mount);
      discard_mntfs(mf);

    } else {

      if (mf->mf_flags & MFF_RESTART) {
	dlog("Discarding remount hook for %s", mf->mf_mount);
      } else {
	dlog("Discarding last mntfs reference to %s fstype %s",
	     mf->mf_mount, mf->mf_ops->fs_type);
      }
      if (mf->mf_flags & (MFF_MOUNTED | MFF_MOUNTING | MFF_UNMOUNTING))
	dlog("mntfs reference for %s still active", mf->mf_mount);
      mf->mf_cid = timeout(ALLOWED_MOUNT_TIME, discard_mntfs, (voidp) mf);
    }
  }
}


mntfs *
realloc_mntfs(mntfs *mf, am_ops *ops, am_opts *mo, char *mp, char *info, char *auto_opts, char *mopts, char *remopts)
{
  mntfs *mf2;

  if (mf->mf_refc == 1 &&
      mf->mf_flags & MFF_RESTART &&
      STREQ(mf->mf_mount, mp)) {
    /*
     * If we are inheriting then just return
     * the same node...
     */
    return mf;
  }

  /*
   * Re-use the existing mntfs if it is mounted.
   * This traps a race in nfsx.
   */
  if (mf->mf_ops != &amfs_error_ops &&
      (mf->mf_flags & MFF_MOUNTED) &&
      !FSRV_ISDOWN(mf->mf_server)) {
    mf->mf_fo = mo;
    return mf;
  }

  mf2 = find_mntfs(ops, mo, mp, info, auto_opts, mopts, remopts);
  free_mntfs(mf);
  return mf2;
}