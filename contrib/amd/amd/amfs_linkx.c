
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
 * Symbol-link file system, with test that the target of the link exists.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amd.h>

/* forward declarations */
static int amfs_linkx_mount(am_node *mp, mntfs *mf);
static int amfs_linkx_umount(am_node *mp, mntfs *mf);

/*
 * linkx operations
 */
struct am_ops amfs_linkx_ops =
{
  "linkx",
  amfs_link_match,
  0,				/* amfs_linkx_init */
  amfs_linkx_mount,
  amfs_linkx_umount,
  amfs_error_lookup_child,
  amfs_error_mount_child,
  amfs_error_readdir,
  0,				/* amfs_linkx_readlink */
  0,				/* amfs_linkx_mounted */
  0,				/* amfs_linkx_umounted */
  amfs_generic_find_srvr,
  0,				/* amfs_linkx_get_wchan */
  FS_MBACKGROUND,
#ifdef HAVE_FS_AUTOFS
  AUTOFS_LINKX_FS_FLAGS,
#endif /* HAVE_FS_AUTOFS */
};


static int
amfs_linkx_mount(am_node *mp, mntfs *mf)
{
  /*
   * Check for existence of target.
   */
  struct stat stb;
  char *ln;

  if (mp->am_link)
    ln = mp->am_link;
  else				/* should never occur */
    ln = mf->mf_mount;

  /*
   * Use lstat, not stat, since we don't
   * want to know if the ultimate target of
   * a symlink chain exists, just the first.
   */
  if (lstat(ln, &stb) < 0)
    return errno;

  return 0;
}


static int
amfs_linkx_umount(am_node *mp, mntfs *mf)
{
  return 0;
}