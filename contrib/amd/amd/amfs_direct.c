
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
 * Direct file system
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amd.h>

/****************************************************************************
 *** FORWARD DEFINITIONS                                                  ***
 ****************************************************************************/
static am_node *amfs_direct_readlink(am_node *mp, int *error_return);

/****************************************************************************
 *** OPS STRUCTURES                                                       ***
 ****************************************************************************/
am_ops amfs_direct_ops =
{
  "direct",
  amfs_generic_match,
  0,				/* amfs_direct_init */
  amfs_toplvl_mount,
  amfs_toplvl_umount,
  amfs_generic_lookup_child,
  amfs_generic_mount_child,
  amfs_error_readdir,
  amfs_direct_readlink,
  amfs_generic_mounted,
  0,				/* amfs_direct_umounted */
  amfs_generic_find_srvr,
  0,				/* amfs_direct_get_wchan */
  FS_DIRECT | FS_MKMNT | FS_NOTIMEOUT | FS_BACKGROUND | FS_AMQINFO,
#ifdef HAVE_FS_AUTOFS
  AUTOFS_DIRECT_FS_FLAGS,
#endif /* HAVE_FS_AUTOFS */
};


/****************************************************************************
 *** FUNCTIONS                                                             ***
 ****************************************************************************/

static am_node *
amfs_direct_readlink(am_node *mp, int *error_return)
{
  am_node *xp;
  int rc = 0;

  xp = next_nonerror_node(mp->am_child);
  if (!xp) {
    if (!mp->am_mnt->mf_private)
      amfs_mkcacheref(mp->am_mnt);	/* XXX */
    xp = amfs_generic_lookup_child(mp, mp->am_path + 1, &rc, VLOOK_CREATE);
    if (xp && rc < 0)
      xp = amfs_generic_mount_child(xp, &rc);
  }
  if (xp) {
    new_ttl(xp);		/* (7/12/89) from Rein Tollevik */
    return xp;
  }
  if (amd_state == Finishing)
    rc = ENOENT;
  *error_return = rc;
  return 0;
}