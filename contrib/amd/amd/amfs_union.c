
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
 * Union automounter file system
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amd.h>

/****************************************************************************
 *** FORWARD DEFINITIONS                                                  ***
 ****************************************************************************/
static int create_amfs_union_node(char *dir, opaque_t arg);
static void amfs_union_mounted(mntfs *mf);


/****************************************************************************
 *** OPS STRUCTURES                                                       ***
 ****************************************************************************/
am_ops amfs_union_ops =
{
  "union",
  amfs_generic_match,
  0,				/* amfs_union_init */
  amfs_toplvl_mount,
  amfs_toplvl_umount,
  amfs_generic_lookup_child,
  amfs_generic_mount_child,
  amfs_generic_readdir,
  0,				/* amfs_union_readlink */
  amfs_union_mounted,
  0,				/* amfs_union_umounted */
  amfs_generic_find_srvr,
  0,				/* amfs_union_get_wchan */
  FS_MKMNT | FS_NOTIMEOUT | FS_BACKGROUND | FS_AMQINFO | FS_DIRECTORY,
#ifdef HAVE_FS_AUTOFS
  AUTOFS_UNION_FS_FLAGS,
#endif /* HAVE_FS_AUTOFS */
};


/*
 * Create a reference to a union'ed entry
 * XXX: this function may not be used anywhere...
 */
static int
create_amfs_union_node(char *dir, opaque_t arg)
{
  if (!STREQ(dir, "/defaults")) {
    int error = 0;
    am_node *am;
    am = amfs_generic_lookup_child(arg, dir, &error, VLOOK_CREATE);
    if (am && error < 0)
      am = amfs_generic_mount_child(am, &error);
    if (error > 0) {
      errno = error;		/* XXX */
      plog(XLOG_ERROR, "unionfs: could not mount %s: %m", dir);
    }
    return error;
  }
  return 0;
}


static void
amfs_union_mounted(mntfs *mf)
{
  int index;
  am_node *mp;

  amfs_mkcacheref(mf);

  /*
   * Having made the union mount point,
   * populate all the entries...
   */
  for (mp = get_first_exported_ap(&index);
       mp;
       mp = get_next_exported_ap(&index)) {
    if (mp->am_mnt == mf) {
      /* return value from create_amfs_union_node is ignored by mapc_keyiter */
      (void) mapc_keyiter((mnt_map *) mp->am_mnt->mf_private,
			  create_amfs_union_node,
			  mp);
      break;
    }
  }
}