
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
 * Symbol-link file system
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amd.h>

/* forward declarations */
static int amfs_link_mount(am_node *mp, mntfs *mf);
static int amfs_link_umount(am_node *mp, mntfs *mf);

/*
 * Ops structures
 */
am_ops amfs_link_ops =
{
  "link",
  amfs_link_match,
  0,				/* amfs_link_init */
  amfs_link_mount,
  amfs_link_umount,
  amfs_error_lookup_child,
  amfs_error_mount_child,
  amfs_error_readdir,
  0,				/* amfs_link_readlink */
  0,				/* amfs_link_mounted */
  0,				/* amfs_link_umounted */
  amfs_generic_find_srvr,
  0,				/* nfs_fs_flags */
  0,				/* amfs_link_get_wchan */
#ifdef HAVE_FS_AUTOFS
  AUTOFS_LINK_FS_FLAGS,
#endif /* HAVE_FS_AUTOFS */
};


/*
 * SFS needs a link.
 */
char *
amfs_link_match(am_opts *fo)
{

  if (!fo->opt_fs) {
    plog(XLOG_USER, "link: no fs specified");
    return 0;
  }

  /*
   * If the link target points to another mount point, then we could
   * end up with an unpleasant situation, where the link f/s simply
   * "assumes" the mntfs of that mount point.
   *
   * For example, if the link points to /usr, and /usr is a real ufs
   * filesystem, then the link f/s will use the inherited ufs mntfs,
   * and the end result will be that it will become unmountable.
   *
   * To prevent this, we use a hack: we prepend a dot ('.') to opt_fs if
   * its original value was an absolute path, so that it will never match
   * any other mntfs.
   *
   * XXX: a less hacky solution should be used...
   */
  if (fo->opt_fs[0] == '/') {
    char *link_hack = str3cat(NULL, ".", fo->opt_fs, "");
    if (!fo->opt_sublink)
      fo->opt_sublink = strdup(fo->opt_fs);
    XFREE(fo->opt_fs);
    fo->opt_fs = link_hack;
  }

  return strdup(fo->opt_fs);
}


static int
amfs_link_mount(am_node *mp, mntfs *mf)
{
  return 0;
}


static int
amfs_link_umount(am_node *mp, mntfs *mf)
{
  return 0;
}