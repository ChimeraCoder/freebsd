
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

/*
 * Irix UN*X file system: XFS (Extended File System)
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amd.h>

/* forward declarations */
static char *xfs_match(am_opts *fo);
static int xfs_mount(am_node *am, mntfs *mf);
static int xfs_umount(am_node *am, mntfs *mf);

/*
 * Ops structure
 */
am_ops xfs_ops =
{
  "xfs",
  xfs_match,
  0,				/* xfs_init */
  xfs_mount,
  xfs_umount,
  amfs_error_lookup_child,
  amfs_error_mount_child,
  amfs_error_readdir,
  0,				/* xfs_readlink */
  0,				/* xfs_mounted */
  0,				/* xfs_umounted */
  amfs_generic_find_srvr,
  0,				/* xfs_get_wchan */
  FS_MKMNT | FS_NOTIMEOUT | FS_UBACKGROUND | FS_AMQINFO, /* nfs_fs_flags */
#ifdef HAVE_FS_AUTOFS
  AUTOFS_XFS_FS_FLAGS,
#endif /* HAVE_FS_AUTOFS */
};


/*
 * XFS needs local filesystem and device.
 */
static char *
xfs_match(am_opts *fo)
{

  if (!fo->opt_dev) {
    plog(XLOG_USER, "xfs: no device specified");
    return 0;
  }

  dlog("XFS: mounting device \"%s\" on \"%s\"", fo->opt_dev, fo->opt_fs);

  /*
   * Determine magic cookie to put in mtab
   */
  return strdup(fo->opt_dev);
}


static int
mount_xfs(char *mntdir, char *fs_name, char *opts, int on_autofs)
{
  xfs_args_t xfs_args;
  mntent_t mnt;
  int flags;

  /*
   * Figure out the name of the file system type.
   */
  MTYPE_TYPE type = MOUNT_TYPE_XFS;

  memset((voidp) &xfs_args, 0, sizeof(xfs_args)); /* Paranoid */

  /*
   * Fill in the mount structure
   */
  memset((voidp) &mnt, 0, sizeof(mnt));
  mnt.mnt_dir = mntdir;
  mnt.mnt_fsname = fs_name;
  mnt.mnt_type = MNTTAB_TYPE_XFS;
  mnt.mnt_opts = opts;

  flags = compute_mount_flags(&mnt);
#ifdef HAVE_FS_AUTOFS
  if (on_autofs)
    flags |= autofs_compute_mount_flags(&mnt);
#endif /* HAVE_FS_AUTOFS */

#ifdef HAVE_XFS_ARGS_T_FLAGS
  xfs_args.flags = 0;		/* XXX: fix this to correct flags */
#endif /* HAVE_XFS_ARGS_T_FLAGS */
#ifdef HAVE_XFS_ARGS_T_FSPEC
  xfs_args.fspec = fs_name;
#endif /* HAVE_XFS_ARGS_T_FSPEC */

  /*
   * Call generic mount routine
   */
  return mount_fs(&mnt, flags, (caddr_t) &xfs_args, 0, type, 0, NULL, mnttab_file_name, on_autofs);
}


static int
xfs_mount(am_node *am, mntfs *mf)
{
  int on_autofs = mf->mf_flags & MFF_ON_AUTOFS;
  int error;

  error = mount_xfs(mf->mf_mount, mf->mf_info, mf->mf_mopts, on_autofs);
  if (error) {
    errno = error;
    plog(XLOG_ERROR, "mount_xfs: %m");
    return error;
  }

  return 0;
}


static int
xfs_umount(am_node *am, mntfs *mf)
{
  int unmount_flags = (mf->mf_flags & MFF_ON_AUTOFS) ? AMU_UMOUNT_AUTOFS : 0;

  return UMOUNT_FS(mf->mf_mount, mnttab_file_name, unmount_flags);
}