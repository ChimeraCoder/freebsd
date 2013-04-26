
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
 * Automount file system
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amd.h>

/****************************************************************************
 *** MACROS                                                               ***
 ****************************************************************************/


/****************************************************************************
 *** STRUCTURES                                                           ***
 ****************************************************************************/


/****************************************************************************
 *** FORWARD DEFINITIONS                                                  ***
 ****************************************************************************/
static int amfs_auto_mount(am_node *mp, mntfs *mf);


/****************************************************************************
 *** OPS STRUCTURES                                                       ***
 ****************************************************************************/
am_ops amfs_auto_ops =
{
  "auto",
  amfs_generic_match,
  0,				/* amfs_auto_init */
  amfs_auto_mount,
  amfs_generic_umount,
  amfs_generic_lookup_child,
  amfs_generic_mount_child,
  amfs_generic_readdir,
  0,				/* amfs_auto_readlink */
  amfs_generic_mounted,
  0,				/* amfs_auto_umounted */
  amfs_generic_find_srvr,
  0,				/* amfs_auto_get_wchan */
  FS_AMQINFO | FS_DIRECTORY,
#ifdef HAVE_FS_AUTOFS
  AUTOFS_AUTO_FS_FLAGS,
#endif /* HAVE_FS_AUTOFS */
};


/****************************************************************************
 *** FUNCTIONS                                                             ***
 ****************************************************************************/
/*
 * Mount a sub-mount
 */
static int
amfs_auto_mount(am_node *mp, mntfs *mf)
{
  /*
   * Pseudo-directories are used to provide some structure
   * to the automounted directories instead
   * of putting them all in the top-level automount directory.
   *
   * Here, just increment the parent's link count.
   */
  mp->am_parent->am_fattr.na_nlink++;

  /*
   * Info field of . means use parent's info field.
   * Historical - not documented.
   */
  if (mf->mf_info[0] == '.' && mf->mf_info[1] == '\0')
    mf->mf_info = strealloc(mf->mf_info, mp->am_parent->am_mnt->mf_info);

  /*
   * Compute prefix:
   *
   * If there is an option prefix then use that else
   * If the parent had a prefix then use that with name
   *      of this node appended else
   * Use the name of this node.
   *
   * That means if you want no prefix you must say so
   * in the map.
   */
  if (mf->mf_fo->opt_pref) {
    /* allow pref:=null to set a real null prefix */
    if (STREQ(mf->mf_fo->opt_pref, "null")) {
      mp->am_pref = strdup("");
    } else {
      /*
       * the prefix specified as an option
       */
      mp->am_pref = strdup(mf->mf_fo->opt_pref);
    }
  } else {
    /*
     * else the parent's prefix
     * followed by the name
     * followed by /
     */
    char *ppref = mp->am_parent->am_pref;
    if (ppref == 0)
      ppref = "";
    mp->am_pref = str3cat((char *) 0, ppref, mp->am_name, "/");
  }

#ifdef HAVE_FS_AUTOFS
  if (mf->mf_flags & MFF_IS_AUTOFS) {
    char opts[SIZEOF_OPTS];
    int error;

    autofs_get_opts(opts, sizeof(opts), mp->am_autofs_fh);

    /* now do the mount */
    error = amfs_mount(mp, mf, opts);
    if (error) {
      errno = error;
      plog(XLOG_FATAL, "amfs_auto_mount: amfs_mount failed: %m");
      return error;
    }
  }
#endif /* HAVE_FS_AUTOFS */

  /*
   * Attach a map cache
   */
  amfs_mkcacheref(mf);

  return 0;
}