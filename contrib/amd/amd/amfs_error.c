
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
 * Error file system.
 * This is used as a last resort catchall if
 * nothing else worked.  EFS just returns lots
 * of error codes, except for unmount which
 * always works of course.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amd.h>

static char *amfs_error_match(am_opts *fo);
static int amfs_error_mount(am_node *am, mntfs *mf);
static int amfs_error_umount(am_node *am, mntfs *mf);


/*
 * Ops structure
 */
am_ops amfs_error_ops =
{
  "error",
  amfs_error_match,
  0,				/* amfs_error_init */
  amfs_error_mount,
  amfs_error_umount,
  amfs_error_lookup_child,
  amfs_error_mount_child,
  amfs_error_readdir,
  0,				/* amfs_error_readlink */
  0,				/* amfs_error_mounted */
  0,				/* amfs_error_umounted */
  amfs_generic_find_srvr,
  0,				/* amfs_error_get_wchan */
  FS_DISCARD,			/* nfs_fs_flags */
#ifdef HAVE_FS_AUTOFS
  AUTOFS_ERROR_FS_FLAGS,
#endif /* HAVE_FS_AUTOFS */
};



/*
 * EFS file system always matches
 */
static char *
amfs_error_match(am_opts *fo)
{
  return strdup("(error-hook)");
}


static int
amfs_error_mount(am_node *am, mntfs *mf)
{
  return ENOENT;
}


static int
amfs_error_umount(am_node *am, mntfs *mf)
{
  /*
   * Always succeed
   */
  return 0;
}


/*
 * EFS interface to RPC lookup() routine.
 * Should never get here in the automounter.
 * If we do then just give an error.
 */
am_node *
amfs_error_lookup_child(am_node *mp, char *fname, int *error_return, int op)
{
  *error_return = ESTALE;
  return 0;
}


/*
 * EFS interface to RPC lookup() routine.
 * Should never get here in the automounter.
 * If we do then just give an error.
 */
am_node *
amfs_error_mount_child(am_node *ap, int *error_return)
{
  *error_return = ESTALE;
  return 0;
}


/*
 * EFS interface to RPC readdir() routine.
 * Should never get here in the automounter.
 * If we do then just give an error.
 */
int
amfs_error_readdir(am_node *mp, nfscookie cookie, nfsdirlist *dp, nfsentry *ep, u_int count)
{
  return ESTALE;
}