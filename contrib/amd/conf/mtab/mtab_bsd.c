
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
 * BSD 4.4 systems don't write their mount tables on a file.  Instead, they
 * use a (better) system where the kernel keeps this state, and you access
 * the mount tables via a known interface.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amu.h>


static mntent_t *
mnt_dup(struct statfs *mp)
{
  mntent_t *new_mp = ALLOC(mntent_t);
  char *ty;

  new_mp->mnt_fsname = strdup(mp->f_mntfromname);
  new_mp->mnt_dir = strdup(mp->f_mntonname);

#ifdef HAVE_STRUCT_STATFS_F_FSTYPENAME
  ty = mp->f_fstypename;
#else /* not HAVE_STRUCT_STATFS_F_FSTYPENAME */
  switch (mp->f_type) {

# if defined(MOUNT_UFS) && defined(MNTTAB_TYPE_UFS)
  case MOUNT_UFS:
    ty = MNTTAB_TYPE_UFS;
    break;
# endif /* defined(MOUNT_UFS) && defined(MNTTAB_TYPE_UFS) */

# if defined(MOUNT_NFS) && defined(MNTTAB_TYPE_NFS)
  case MOUNT_NFS:
    ty = MNTTAB_TYPE_NFS;
    break;
# endif /* defined(MOUNT_NFS) && defined(MNTTAB_TYPE_NFS) */

# if defined(MOUNT_MFS) && defined(MNTTAB_TYPE_MFS)
  case MOUNT_MFS:
    ty = MNTTAB_TYPE_MFS;
    break;
# endif /* defined(MOUNT_MFS) && defined(MNTTAB_TYPE_MFS) */

  default:
    ty = "unknown";

    break;
  }
#endif /* not HAVE_STRUCT_STATFS_F_FSTYPENAME */

  new_mp->mnt_type = strdup(ty);
  new_mp->mnt_opts = strdup("unset");
  new_mp->mnt_freq = 0;
  new_mp->mnt_passno = 0;

  return new_mp;
}


/*
 * Read a mount table into memory
 */
mntlist *
read_mtab(char *fs, const char *mnttabname)
{
  mntlist **mpp, *mhp;
  struct statfs *mntbufp, *mntp;

  int nloc = getmntinfo(&mntbufp, MNT_NOWAIT);

  if (nloc == 0) {
    plog(XLOG_ERROR, "Can't read mount table");
    return 0;
  }
  mpp = &mhp;
  for (mntp = mntbufp; mntp < mntbufp + nloc; mntp++) {
    /*
     * Allocate a new slot
     */
    *mpp = ALLOC(struct mntlist);

    /*
     * Copy the data returned by getmntent
     */
    (*mpp)->mnt = mnt_dup(mntp);

    /*
     * Move to next pointer
     */
    mpp = &(*mpp)->mnext;
  }

  /*
   * Terminate the list
   */
  *mpp = 0;

  return mhp;
}