
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
 * Loopback file system
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amd.h>

/* forward definitions */
static char *lofs_match(am_opts *fo);
static int lofs_mount(am_node *am, mntfs *mf);
static int lofs_umount(am_node *am, mntfs *mf);

/*
 * Ops structure
 */
am_ops lofs_ops =
{
  "lofs",
  lofs_match,
  0,				/* lofs_init */
  lofs_mount,
  lofs_umount,
  amfs_error_lookup_child,
  amfs_error_mount_child,
  amfs_error_readdir,
  0,				/* lofs_readlink */
  0,				/* lofs_mounted */
  0,				/* lofs_umounted */
  amfs_generic_find_srvr,
  0,				/* lofs_get_wchan */
  FS_MKMNT | FS_NOTIMEOUT | FS_UBACKGROUND | FS_AMQINFO, /* nfs_fs_flags */
#ifdef HAVE_FS_AUTOFS
  AUTOFS_LOFS_FS_FLAGS,
#endif /* HAVE_FS_AUTOFS */
};


/*
 * LOFS needs remote filesystem.
 */
static char *
lofs_match(am_opts *fo)
{
  if (!fo->opt_rfs) {
    plog(XLOG_USER, "lofs: no source filesystem specified");
    return 0;
  }
  dlog("LOFS: mounting fs \"%s\" on \"%s\"",
       fo->opt_rfs, fo->opt_fs);

  /*
   * Determine magic cookie to put in mtab
   */
  return strdup(fo->opt_rfs);
}


int
mount_lofs(char *mntdir, char *fs_name, char *opts, int on_autofs)
{
  mntent_t mnt;
  int flags;

  /*
   * Figure out the name of the file system type.
   */
  MTYPE_TYPE type = MOUNT_TYPE_LOFS;

  /*
   * Fill in the mount structure
   */
  memset((voidp) &mnt, 0, sizeof(mnt));
  mnt.mnt_dir = mntdir;
  mnt.mnt_fsname = fs_name;
  mnt.mnt_type = MNTTAB_TYPE_LOFS;
  mnt.mnt_opts = opts;

  flags = compute_mount_flags(&mnt);
#ifdef HAVE_FS_AUTOFS
  if (on_autofs)
    flags |= autofs_compute_mount_flags(&mnt);
#endif /* HAVE_FS_AUTOFS */

  /*
   * Call generic mount routine
   */
  return mount_fs(&mnt, flags, NULL, 0, type, 0, NULL, mnttab_file_name, on_autofs);
}


static int
lofs_mount(am_node *am, mntfs *mf)
{
  int on_autofs = mf->mf_flags & MFF_ON_AUTOFS;
  int error;

  error = mount_lofs(mf->mf_mount, mf->mf_info, mf->mf_mopts, on_autofs);
  if (error) {
    errno = error;
    plog(XLOG_ERROR, "mount_lofs: %m");
    return error;
  }
  return 0;
}


static int
lofs_umount(am_node *am, mntfs *mf)
{
  int unmount_flags = (mf->mf_flags & MFF_ON_AUTOFS) ? AMU_UMOUNT_AUTOFS : 0;

  return UMOUNT_FS(mf->mf_mount, mnttab_file_name, unmount_flags);
}