
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
 * PC (MS-DOS) file system
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amd.h>

/* forward definitions */
static char *pcfs_match(am_opts *fo);
static int pcfs_mount(am_node *am, mntfs *mf);
static int pcfs_umount(am_node *am, mntfs *mf);

/*
 * Ops structure
 */
am_ops pcfs_ops =
{
  "pcfs",
  pcfs_match,
  0,				/* pcfs_init */
  pcfs_mount,
  pcfs_umount,
  amfs_error_lookup_child,
  amfs_error_mount_child,
  amfs_error_readdir,
  0,				/* pcfs_readlink */
  0,				/* pcfs_mounted */
  0,				/* pcfs_umounted */
  amfs_generic_find_srvr,
  0,				/* pcfs_get_wchan */
  FS_MKMNT | FS_UBACKGROUND | FS_AMQINFO,	/* nfs_fs_flags */
#ifdef HAVE_FS_AUTOFS
  AUTOFS_PCFS_FS_FLAGS,
#endif /* HAVE_FS_AUTOFS */
};



/*
 * PCFS needs remote filesystem.
 */
static char *
pcfs_match(am_opts *fo)
{
  if (!fo->opt_dev) {
    plog(XLOG_USER, "pcfs: no source device specified");
    return 0;
  }
  dlog("PCFS: mounting device \"%s\" on \"%s\"", fo->opt_dev, fo->opt_fs);

  /*
   * Determine magic cookie to put in mtab
   */
  return strdup(fo->opt_dev);
}


static int
mount_pcfs(char *mntdir, char *fs_name, char *opts, int on_autofs)
{
  pcfs_args_t pcfs_args;
  mntent_t mnt;
  int flags;
#if defined(HAVE_PCFS_ARGS_T_MASK) || defined(HAVE_PCFS_ARGS_T_DIRMASK)
  int mask;
#endif /* defined(HAVE_PCFS_ARGS_T_MASK) || defined(HAVE_PCFS_ARGS_T_DIRMASK) */
#if defined(HAVE_PCFS_ARGS_T_UID) || defined(HAVE_PCFS_ARGS_T_UID)
  char *str;
#endif /* defined(HAVE_PCFS_ARGS_T_UID) || defined(HAVE_PCFS_ARGS_T_UID) */

  /*
   * Figure out the name of the file system type.
   */
  MTYPE_TYPE type = MOUNT_TYPE_PCFS;

  memset((voidp) &pcfs_args, 0, sizeof(pcfs_args)); /* Paranoid */

  /*
   * Fill in the mount structure
   */
  memset((voidp) &mnt, 0, sizeof(mnt));
  mnt.mnt_dir = mntdir;
  mnt.mnt_fsname = fs_name;
  mnt.mnt_type = MNTTAB_TYPE_PCFS;
  mnt.mnt_opts = opts;

  flags = compute_mount_flags(&mnt);
#ifdef HAVE_FS_AUTOFS
  if (on_autofs)
    flags |= autofs_compute_mount_flags(&mnt);
#endif /* HAVE_FS_AUTOFS */
  if (amuDebug(D_TRACE))
    plog(XLOG_DEBUG, "mount_pcfs: flags=0x%x", (u_int) flags);

#ifdef HAVE_PCFS_ARGS_T_FSPEC
  pcfs_args.fspec = fs_name;
#endif /* HAVE_PCFS_ARGS_T_FSPEC */

#ifdef HAVE_PCFS_ARGS_T_MASK
  pcfs_args.mask = 0777;	/* this may be the msdos file modes */
  if ((mask = hasmntval(&mnt, MNTTAB_OPT_MASK)) > 0)
    pcfs_args.mask = mask;
  if (amuDebug(D_TRACE))
    plog(XLOG_DEBUG, "mount_pcfs: mask=%o (octal)", (u_int) pcfs_args.mask);
#endif /* HAVE_PCFS_ARGS_T_MASK */

#ifdef HAVE_PCFS_ARGS_T_DIRMASK
  pcfs_args.dirmask = 0777;    /* this may be the msdos dir modes */
  if ((mask = hasmntval(&mnt, MNTTAB_OPT_DIRMASK)) > 0)
    pcfs_args.dirmask = mask;
  if (amuDebug(D_TRACE))
    plog(XLOG_DEBUG, "mount_pcfs: dirmask=%o (octal)", (u_int) pcfs_args.dirmask);
#endif /* HAVE_PCFS_ARGS_T_DIRMASK */

#ifdef HAVE_PCFS_ARGS_T_UID
  pcfs_args.uid = 0;		/* default to root */
  if ((str = hasmntstr(&mnt, MNTTAB_OPT_USER)) != NULL) {
    struct passwd *pw;
    if ((pw = getpwnam(str)) != NULL)
      pcfs_args.uid = pw->pw_uid;
    else		 /* maybe used passed a UID number, not user name */
      pcfs_args.uid = atoi(str); /* atoi returns '0' if it failed */
    XFREE(str);
  }
  if (amuDebug(D_TRACE))
    plog(XLOG_DEBUG, "mount_pcfs: uid=%d", (int) pcfs_args.uid);
#endif /* HAVE_PCFS_ARGS_T_UID */

#ifdef HAVE_PCFS_ARGS_T_GID
  pcfs_args.gid = 0;		/* default to wheel/root group */
  if ((str = hasmntstr(&mnt, MNTTAB_OPT_GROUP)) != NULL) {
    struct group *gr;
    if ((gr = getgrnam(str)) != NULL)
      pcfs_args.gid = gr->gr_gid;
    else		/* maybe used passed a GID number, not group name */
      pcfs_args.gid = atoi(str); /* atoi returns '0' if it failed */
    XFREE(str);
  }
  if (amuDebug(D_TRACE))
    plog(XLOG_DEBUG, "mount_pcfs: gid=%d", (int) pcfs_args.gid);
#endif /* HAVE_PCFS_ARGS_T_GID */

#ifdef HAVE_PCFS_ARGS_T_SECONDSWEST
  pcfs_args.secondswest = 0;	/* XXX: fill in correct values */
#endif /* HAVE_PCFS_ARGS_T_SECONDSWEST */
#ifdef HAVE_PCFS_ARGS_T_DSTTIME
  pcfs_args.dsttime = 0;	/* XXX: fill in correct values */
#endif /* HAVE_PCFS_ARGS_T_DSTTIME */

  /*
   * Call generic mount routine
   */
  return mount_fs(&mnt, flags, (caddr_t) & pcfs_args, 0, type, 0, NULL, mnttab_file_name, on_autofs);
}


static int
pcfs_mount(am_node *am, mntfs *mf)
{
  int on_autofs = mf->mf_flags & MFF_ON_AUTOFS;
  int error;

  error = mount_pcfs(mf->mf_mount, mf->mf_info, mf->mf_mopts, on_autofs);
  if (error) {
    errno = error;
    plog(XLOG_ERROR, "mount_pcfs: %m");
    return error;
  }

  return 0;
}


static int
pcfs_umount(am_node *am, mntfs *mf)
{
  int unmount_flags = (mf->mf_flags & MFF_ON_AUTOFS) ? AMU_UMOUNT_AUTOFS : 0;

  return UMOUNT_FS(mf->mf_mount, mnttab_file_name, unmount_flags);
}