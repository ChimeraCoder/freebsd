
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amd.h>


static void
restart_fake_mntfs(mntent_t *me, am_ops *fs_ops)
{
  mntfs *mf;
  am_opts mo;
  char *cp;

  /*
   * Partially fake up an opts structure
   */
  memset(&mo, 0, sizeof(mo));
  mo.opt_rhost = 0;
  mo.opt_rfs = 0;
  cp = strchr(me->mnt_fsname, ':');
  if (cp) {
    *cp = '\0';
    mo.opt_rhost = strdup(me->mnt_fsname);
    mo.opt_rfs = strdup(cp + 1);
    *cp = ':';
  } else if (STREQ(me->mnt_type, MNTTAB_TYPE_NFS)) {
    /*
     * Hacky workaround for mnttab NFS entries that only list the server
     */
    plog(XLOG_WARNING, "NFS server entry assumed to be %s:/", me->mnt_fsname);
    mo.opt_rhost = strdup(me->mnt_fsname);
    mo.opt_rfs = strdup("/");
    me->mnt_fsname = str3cat(me->mnt_fsname, mo.opt_rhost, ":", "/");
  }
  mo.opt_fs = me->mnt_dir;
  mo.opt_opts = me->mnt_opts;

  /*
   * Make a new mounted filesystem
   */
  mf = find_mntfs(fs_ops, &mo, me->mnt_dir,
		  me->mnt_fsname, "", me->mnt_opts, "");
  if (mf->mf_refc == 1) {
    mf->mf_flags |= MFF_RESTART | MFF_MOUNTED;
    mf->mf_error = 0;		     /* Already mounted correctly */
    mf->mf_fo = 0;
    /*
     * Only timeout non-NFS entries
     */
    if (!STREQ(me->mnt_type, MNTTAB_TYPE_NFS))
      mf->mf_flags |= MFF_RSTKEEP;
    if (fs_ops->fs_init) {
      /*
       * Don't care whether this worked since
       * it is checked again when the fs is
       * inherited.
       */
      (void) (*fs_ops->fs_init) (mf);
    }
    plog(XLOG_INFO, "%s restarted fstype %s on %s, flags 0x%x",
	 me->mnt_fsname, fs_ops->fs_type, me->mnt_dir, mf->mf_flags);
  } else {
    /* Something strange happened - two mounts at the same place! */
    free_mntfs(mf);
  }
  /*
   * Clean up mo
   */
  if (mo.opt_rhost)
    XFREE(mo.opt_rhost);
  if (mo.opt_rfs)
    XFREE(mo.opt_rfs);
}


/*
 * Handle an amd restart.
 *
 * Scan through the mount list finding all "interesting" mount points.
 * Next hack up partial data structures and add the mounted file
 * system to the list of known filesystems.
 *
 * This module relies on internal details of other components.  If
 * you change something else make *sure* restart() still works.
 */
void
restart(void)
{
  mntlist *ml, *mlp;

  /*
   * Read the existing mount table.  For each entry, find nfs, ufs or auto
   * mounts and create a partial am_node to represent it.
   */
  for (mlp = ml = read_mtab("restart", mnttab_file_name);
       mlp;
       mlp = mlp->mnext) {
    mntent_t *me = mlp->mnt;
    am_ops *fs_ops = 0;

    if (STREQ(me->mnt_type, MNTTAB_TYPE_NFS)) {
      /*
       * NFS entry, or possibly an Amd entry...
       * The mnt_fsname for daemon mount points is
       * 	host:(pidXXX)
       * or (seen on Solaris)
       *        host:daemon(pidXXX)
       */
      char *colon = strchr(me->mnt_fsname, ':');
      if (colon && strstr(colon, "(pid"))
	continue;
    }

    /* Search for the correct filesystem ops */
    fs_ops = ops_search(me->mnt_type);

    /*
     * Catch everything else with symlinks to
     * avoid recursive mounts.  This is debatable...
     */
    if (!fs_ops)
      fs_ops = &amfs_link_ops;

    restart_fake_mntfs(me, fs_ops);
  }

  /*
   * Free the mount list
   */
  free_mntlist(ml);
}


/*
 * Handle an amd restart for amd's own mount points.
 *
 * Scan through the mount list finding all daemon mount points
 * (determined by the presence of a pid inside the mount info).
 * Next hack up partial data structures and add the mounted file
 * system to the list of known filesystems.
 *
 * This module relies on internal details of other components.  If
 * you change something else make *sure* restart() still works.
 */
void
restart_automounter_nodes(void)
{
  mntlist *ml, *mlp;
  /* reasonably sized list of restarted nfs ports */
  u_short old_ports[256];

  memset((voidp) &old_ports, 0, sizeof(u_short) * 256);

  /*
   * Read the existing mount table.  For each entry, find nfs, ufs or auto
   * mounts and create a partial am_node to represent it.
   */
  for (mlp = ml = read_mtab("restart", mnttab_file_name);
       mlp;
       mlp = mlp->mnext) {
    mntent_t *me = mlp->mnt;
    am_ops *fs_ops = 0;
    char *colon;
    long pid;
    u_short port;
    int err;

    if (!STREQ(me->mnt_type, MNTTAB_TYPE_NFS))
      continue;			/* to next mlp */
    /*
     * NFS entry, or possibly an Amd entry...
     * The mnt_fsname for daemon mount points is
     *	   host:(pidXXX)
     * or (seen on Solaris)
     *     host:daemon(pidXXX)
     */
    colon = strchr(me->mnt_fsname, ':');
    if (!colon || !strstr(colon, "(pid"))
      continue;
    /* if got here, then we matched an existing Amd mount point */
    err = 1;

    plog(XLOG_WARNING, "%s is an existing automount point", me->mnt_dir);

    /* Is the old automounter still alive? */
    if (sscanf(colon, "%*[^(](pid%ld%*[,)]", &pid) != 1) {
      plog(XLOG_WARNING, "Can't parse pid in %s", me->mnt_fsname);
      goto give_up;
    }
    if (kill(pid, 0) != -1 || errno != ESRCH) {
      plog(XLOG_WARNING, "Automounter (pid: %ld) still alive", pid);
      goto give_up;
    }

    /*
     * Do we have a map for this mount point?  Who cares, we'll restart
     * anyway -- getting ESTALE is way better than hanging.
     */

    /* Can we restart it? Only if it tells us what port it was using... */
    if (sscanf(colon, "%*[^,],port%hu)", &port) != 1) {
      plog(XLOG_WARNING, "No port specified for %s", me->mnt_fsname);
      goto give_up;
    }

    /* Maybe we already own that port... */
    if (port != nfs_port) {
      int i;
      for (i = 0; i < 256; i++) {
	if (old_ports[i] == port ||
	    old_ports[i] == 0)
	  break;
      }
      if (i == 256) {
	plog(XLOG_WARNING, "Too many open ports (256)");
	goto give_up;
      }

      if (old_ports[i] == 0) {
	int soNFS;
	SVCXPRT *nfsxprt;
	if (create_nfs_service(&soNFS, &port, &nfsxprt, nfs_program_2) != 0) {
	  plog(XLOG_WARNING, "Can't bind to port %u", port);
	  goto give_up;
	}
	old_ports[i] = nfs_port = port;
      }
    }
    err = 0;

  give_up:
    if (err) {
      plog(XLOG_WARNING, "Can't restart %s, leaving it alone", me->mnt_dir);
      fs_ops = &amfs_link_ops;
    } else {
      fs_ops = &amfs_toplvl_ops;
    }

    restart_fake_mntfs(me, fs_ops);
  } /* end of "for (mlp" */

  /* free the mount list */
  free_mntlist(ml);
}