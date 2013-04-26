
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
 * High Sierra (CD-ROM) file system
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amd.h>

/* forward declarations */
static char *cdfs_match(am_opts *fo);
static int cdfs_mount(am_node *am, mntfs *mf);
static int cdfs_umount(am_node *am, mntfs *mf);

/*
 * Ops structure
 */
am_ops cdfs_ops =
{
  "cdfs",
  cdfs_match,
  0,				/* cdfs_init */
  cdfs_mount,
  cdfs_umount,
  amfs_error_lookup_child,
  amfs_error_mount_child,
  amfs_error_readdir,
  0,				/* cdfs_readlink */
  0,				/* cdfs_mounted */
  0,				/* cdfs_umounted */
  amfs_generic_find_srvr,
  0,				/* cdfs_get_wchan */
  FS_MKMNT | FS_UBACKGROUND | FS_AMQINFO,	/* nfs_fs_flags */
#ifdef HAVE_FS_AUTOFS
  AUTOFS_CDFS_FS_FLAGS,
#endif /* HAVE_FS_AUTOFS */
};


/*
 * CDFS needs remote filesystem.
 */
static char *
cdfs_match(am_opts *fo)
{
  if (!fo->opt_dev) {
    plog(XLOG_USER, "cdfs: no source device specified");
    return 0;
  }
  dlog("CDFS: mounting device \"%s\" on \"%s\"",
       fo->opt_dev, fo->opt_fs);

  /*
   * Determine magic cookie to put in mtab
   */
  return strdup(fo->opt_dev);
}


static int
mount_cdfs(char *mntdir, char *fs_name, char *opts, int on_autofs)
{
  cdfs_args_t cdfs_args;
  mntent_t mnt;
  int genflags, cdfs_flags, retval;

  /*
   * Figure out the name of the file system type.
   */
  MTYPE_TYPE type = MOUNT_TYPE_CDFS;

  memset((voidp) &cdfs_args, 0, sizeof(cdfs_args)); /* Paranoid */
  cdfs_flags = 0;

  /*
   * Fill in the mount structure
   */
  memset((voidp) &mnt, 0, sizeof(mnt));
  mnt.mnt_dir = mntdir;
  mnt.mnt_fsname = fs_name;
  mnt.mnt_type = MNTTAB_TYPE_CDFS;
  mnt.mnt_opts = opts;

#if defined(MNT2_CDFS_OPT_DEFPERM) && defined(MNTTAB_OPT_DEFPERM)
  if (amu_hasmntopt(&mnt, MNTTAB_OPT_DEFPERM))
# ifdef MNT2_CDFS_OPT_DEFPERM
    cdfs_flags |= MNT2_CDFS_OPT_DEFPERM;
# else /* not MNT2_CDFS_OPT_DEFPERM */
    cdfs_flags &= ~MNT2_CDFS_OPT_NODEFPERM;
# endif /* not MNT2_CDFS_OPT_DEFPERM */
#endif /* defined(MNT2_CDFS_OPT_DEFPERM) && defined(MNTTAB_OPT_DEFPERM) */

#if defined(MNT2_CDFS_OPT_NODEFPERM) && defined(MNTTAB_OPT_NODEFPERM)
  if (amu_hasmntopt(&mnt, MNTTAB_OPT_NODEFPERM))
    cdfs_flags |= MNT2_CDFS_OPT_NODEFPERM;
#endif /* MNTTAB_OPT_NODEFPERM */

#if defined(MNT2_CDFS_OPT_NOVERSION) && defined(MNTTAB_OPT_NOVERSION)
  if (amu_hasmntopt(&mnt, MNTTAB_OPT_NOVERSION))
    cdfs_flags |= MNT2_CDFS_OPT_NOVERSION;
#endif /* defined(MNT2_CDFS_OPT_NOVERSION) && defined(MNTTAB_OPT_NOVERSION) */

#if defined(MNT2_CDFS_OPT_RRIP) && defined(MNTTAB_OPT_RRIP)
  if (amu_hasmntopt(&mnt, MNTTAB_OPT_RRIP))
    cdfs_flags |= MNT2_CDFS_OPT_RRIP;
#endif /* defined(MNT2_CDFS_OPT_RRIP) && defined(MNTTAB_OPT_RRIP) */
#if defined(MNT2_CDFS_OPT_NORRIP) && defined(MNTTAB_OPT_NORRIP)
  if (amu_hasmntopt(&mnt, MNTTAB_OPT_NORRIP))
    cdfs_flags |= MNT2_CDFS_OPT_NORRIP;
#endif /* defined(MNT2_CDFS_OPT_NORRIP) && defined(MNTTAB_OPT_NORRIP) */

#if defined(MNT2_CDFS_OPT_GENS) && defined(MNTTAB_OPT_GENS)
  if (amu_hasmntopt(&mnt, MNTTAB_OPT_GENS))
    cdfs_flags |= MNT2_CDFS_OPT_GENS;
#endif /* defined(MNT2_CDFS_OPT_GENS) && defined(MNTTAB_OPT_GENS) */
#if defined(MNT2_CDFS_OPT_EXTATT) && defined(MNTTAB_OPT_EXTATT)
  if (amu_hasmntopt(&mnt, MNTTAB_OPT_EXTATT))
    cdfs_flags |= MNT2_CDFS_OPT_EXTATT;
#endif /* defined(MNT2_CDFS_OPT_EXTATT) && defined(MNTTAB_OPT_EXTATT) */

  genflags = compute_mount_flags(&mnt);
#ifdef HAVE_FS_AUTOFS
  if (on_autofs)
    genflags |= autofs_compute_mount_flags(&mnt);
#endif /* HAVE_FS_AUTOFS */

#ifdef HAVE_CDFS_ARGS_T_FLAGS
  cdfs_args.flags = cdfs_flags;
#endif /* HAVE_CDFS_ARGS_T_FLAGS */

#ifdef HAVE_CDFS_ARGS_T_ISO_FLAGS
  cdfs_args.iso_flags = genflags | cdfs_flags;
#endif /* HAVE_CDFS_ARGS_T_ISO_FLAGS */

#ifdef HAVE_CDFS_ARGS_T_ISO_PGTHRESH
  cdfs_args.iso_pgthresh = hasmntval(&mnt, MNTTAB_OPT_PGTHRESH);
#endif /* HAVE_CDFS_ARGS_T_ISO_PGTHRESH */

#ifdef HAVE_CDFS_ARGS_T_NORRIP
  /* XXX: need to provide norrip mount opt */
  cdfs_args.norrip = 0;		/* use Rock-Ridge Protocol extensions */
#endif /* HAVE_CDFS_ARGS_T_NORRIP */

#ifdef HAVE_CDFS_ARGS_T_SSECTOR
  /* XXX: need to provide ssector mount option */
  cdfs_args.ssector = 0;	/* use 1st session on disk */
#endif /* HAVE_CDFS_ARGS_T_SSECTOR */

#ifdef HAVE_CDFS_ARGS_T_FSPEC
  cdfs_args.fspec = fs_name;
#endif /* HAVE_CDFS_ARGS_T_FSPEC */

  /*
   * Call generic mount routine
   */
  retval = mount_fs(&mnt, genflags, (caddr_t) &cdfs_args, 0, type, 0, NULL, mnttab_file_name, on_autofs);

  return retval;
}


static int
cdfs_mount(am_node *am, mntfs *mf)
{
  int on_autofs = mf->mf_flags & MFF_ON_AUTOFS;
  int error;

  error = mount_cdfs(mf->mf_mount, mf->mf_info, mf->mf_mopts, on_autofs);
  if (error) {
    errno = error;
    plog(XLOG_ERROR, "mount_cdfs: %m");
    return error;
  }
  return 0;
}


static int
cdfs_umount(am_node *am, mntfs *mf)
{
  int unmount_flags = (mf->mf_flags & MFF_ON_AUTOFS) ? AMU_UMOUNT_AUTOFS : 0;

  return UMOUNT_FS(mf->mf_mount, mnttab_file_name, unmount_flags);
}