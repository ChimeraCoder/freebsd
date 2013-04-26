
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
 * Root file system
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amd.h>

/****************************************************************************
 *** FORWARD DEFINITIONS                                                  ***
 ****************************************************************************/
static int amfs_root_mount(am_node *mp, mntfs *mf);

/****************************************************************************
 *** OPS STRUCTURES                                                       ***
 ****************************************************************************/
am_ops amfs_root_ops =
{
  "root",
  0,				/* amfs_root_match */
  0,				/* amfs_root_init */
  amfs_root_mount,
  amfs_generic_umount,
  amfs_generic_lookup_child,
  amfs_generic_mount_child,
  amfs_generic_readdir,
  0,				/* amfs_root_readlink */
  0,				/* amfs_root_mounted */
  0,				/* amfs_root_umounted */
  amfs_generic_find_srvr,
  0,				/* amfs_root_get_wchan */
  FS_NOTIMEOUT | FS_AMQINFO | FS_DIRECTORY,	/* nfs_fs_flags */
#ifdef HAVE_FS_AUTOFS
  AUTOFS_ROOT_FS_FLAGS,
#endif /* HAVE_FS_AUTOFS */
};


/****************************************************************************
 *** FUNCTIONS                                                             ***
 ****************************************************************************/

/*
 * Mount the root...
 */
static int
amfs_root_mount(am_node *mp, mntfs *mf)
{
  mf->mf_mount = strealloc(mf->mf_mount, pid_fsname);
  mf->mf_private = (opaque_t) mapc_find(mf->mf_info, "", NULL);
  mf->mf_prfree = mapc_free;

  return 0;
}