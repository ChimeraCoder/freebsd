
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
 * Program file system
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amd.h>

/* forward definitions */
static char *amfs_program_match(am_opts *fo);
static int amfs_program_mount(am_node *am, mntfs *mf);
static int amfs_program_umount(am_node *am, mntfs *mf);
static int amfs_program_init(mntfs *mf);

/*
 * Ops structure
 */
am_ops amfs_program_ops =
{
  "program",
  amfs_program_match,
  amfs_program_init,
  amfs_program_mount,
  amfs_program_umount,
  amfs_error_lookup_child,
  amfs_error_mount_child,
  amfs_error_readdir,
  0,				/* amfs_program_readlink */
  0,				/* amfs_program_mounted */
  0,				/* amfs_program_umounted */
  amfs_generic_find_srvr,
  0,				/* amfs_program_get_wchan */
  FS_MKMNT | FS_BACKGROUND | FS_AMQINFO,	/* nfs_fs_flags */
#ifdef HAVE_FS_AUTOFS
  AUTOFS_PROGRAM_FS_FLAGS,
#endif /* HAVE_FS_AUTOFS */
};


/*
 * Execute needs a mount and unmount command.
 */
static char *
amfs_program_match(am_opts *fo)
{
  char *prog;

  if (fo->opt_unmount && fo->opt_umount) {
    plog(XLOG_ERROR, "program: cannot specify both unmount and umount options");
    return 0;
  }
  if (!fo->opt_mount) {
    plog(XLOG_ERROR, "program: must specify mount command");
    return 0;
  }
  if (!fo->opt_unmount && !fo->opt_umount) {
    fo->opt_unmount = str3cat(NULL, UNMOUNT_PROGRAM, " umount ", fo->opt_fs);
    plog(XLOG_INFO, "program: un/umount not specified; using default \"%s\"",
	 fo->opt_unmount);
  }
  prog = strchr(fo->opt_mount, ' ');

  return strdup(prog ? prog + 1 : fo->opt_mount);
}


static int
amfs_program_init(mntfs *mf)
{
  /* check if already saved value */
  if (mf->mf_private != NULL)
    return 0;

  /* save unmount (or umount) command */
  if (mf->mf_fo->opt_unmount != NULL)
    mf->mf_private = (opaque_t) strdup(mf->mf_fo->opt_unmount);
  else
    mf->mf_private = (opaque_t) strdup(mf->mf_fo->opt_umount);
  mf->mf_prfree = (void (*)(opaque_t)) free;

  return 0;
}


static int
amfs_program_exec(char *info)
{
  char **xivec;
  int error;

  /*
   * Split copy of command info string
   */
  info = strdup(info);
  if (info == 0)
    return ENOBUFS;
  xivec = strsplit(info, ' ', '\'');

  /*
   * Put stdout to stderr
   */
  (void) fclose(stdout);
  if (!logfp)
    logfp = stderr;		/* initialize before possible first use */
  (void) dup(fileno(logfp));
  if (fileno(logfp) != fileno(stderr)) {
    (void) fclose(stderr);
    (void) dup(fileno(logfp));
  }

  /*
   * Try the exec
   */
  if (amuDebug(D_FULL)) {
    char **cp = xivec;
    plog(XLOG_DEBUG, "executing (un)mount command...");
    while (*cp) {
      plog(XLOG_DEBUG, "arg[%ld] = '%s'", (long) (cp - xivec), *cp);
      cp++;
    }
  }

  if (xivec[0] == 0 || xivec[1] == 0) {
    errno = EINVAL;
    plog(XLOG_USER, "1st/2nd args missing to (un)mount program");
  } else {
    (void) execv(xivec[0], xivec + 1);
  }

  /*
   * Save error number
   */
  error = errno;
  plog(XLOG_ERROR, "exec failed: %m");

  /*
   * Free allocate memory
   */
  XFREE(info);
  XFREE(xivec);

  /*
   * Return error
   */
  return error;
}


static int
amfs_program_mount(am_node *am, mntfs *mf)
{
  return amfs_program_exec(mf->mf_fo->opt_mount);
}


static int
amfs_program_umount(am_node *am, mntfs *mf)
{
  return amfs_program_exec((char *) mf->mf_private);
}