
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
#include <fsi_data.h>
#include <fsinfo.h>


/*
 * Write a sequence of automount mount map entries
 */
static int
write_amount_info(FILE *af, automount *ap,  u_int sk)
{
  int errors = 0;

  if (ap->a_mount) {
    /*
     * A pseudo-directory.
     * This can also be a top-level directory, in which
     * case the type:=auto is not wanted...
     *
     * type:=auto;fs:=${map};pref:=whatever/
     */
    automount *ap2;
    if (strlen(ap->a_name) > sk) {
      fprintf(af, "%s type:=auto;fs:=${map};pref:=%s/\n",
	      ap->a_name + sk, ap->a_name + sk);
    }
    ITER(ap2, automount, ap->a_mount)
      errors += write_amount_info(af, ap2, sk);
  } else if (ap->a_hardwiredfs) {

    /*
     * A hardwired filesystem "hostname:path"
     * rhost:=hostname;rfs:=path
     */
    char *key = ap->a_name + sk;
    char *hostname = ap->a_hardwiredfs;
    char *path = strrchr(hostname, (int) ':');

    if (path == NULL) {
      fprintf(stderr, "%s: %s not an NFS filesystem\n", ap->a_name, ap->a_hardwiredfs);
      errors++;
    } else {
      *path = '\0';
      path++;

      /*
       * Output the map key
       */
      fputs(key, af);
      fprintf(af, " rhost:=%s", hostname);
      fprintf(af, ";rfs:=%s", path);
      if (ap->a_opts && !STREQ(ap->a_opts, "")) {
	fprintf(af, ";%s", ap->a_opts);
      }
      fputc('\n', af);
      path--;
      *path = ':';
    }
  } else if (ap->a_mounted) {

    /*
     * A mounted partition
     * type:=link [ link entries ] type:=nfs [ nfs entries ]
     */
    dict_data *dd;
    dict_ent *de = ap->a_mounted;
    int done_type_link = 0;
    char *key = ap->a_name + sk;

    /*
     * Output the map key
     */
    fputs(key, af);

    /*
     * First output any Link locations that would not
     * otherwise be correctly mounted.  These refer
     * to filesystem which are not mounted in the same
     * place which the automounter would use.
     */
    ITER(dd, dict_data, &de->de_q) {
      fsi_mount *mp = (fsi_mount *) dd->dd_data;
      /*
       * If the mount point and the exported volname are the
       * same then this filesystem will be recognized by
       * the restart code - so we don't need to put out a
       * special rule for it.
       */
      if (mp->m_dk->d_host->h_lochost) {
	char amountpt[1024];
	compute_automount_point(amountpt, sizeof(amountpt),
				mp->m_dk->d_host, mp->m_exported->m_volname);
	if (!STREQ(mp->m_dk->d_mountpt, amountpt)) {
	  /*
	   * ap->a_volname is the name of the aliased volume
	   * mp->m_name is the mount point of the filesystem
	   * mp->m_volname is the volume name of the filesystems
	   */

	  /*
	   * Find length of key and volume names
	   */
	  int avlen = strlen(ap->a_volname);
	  int mnlen = strlen(mp->m_volname);

	  /*
	   * Make sure a -type:=link is output once
	   */
	  if (!done_type_link) {
	    done_type_link = 1;
	    fputs(" -type:=link", af);
	  }

	  /*
	   * Output a selector for the hostname,
	   * the device from which to mount and
	   * where to mount.  This will correspond
	   * to the values output for the fstab.
	   */
	  if (mp->m_dk->d_host->h_lochost)
	    fprintf(af, " host==%s", mp->m_dk->d_host->h_lochost);
	  else
	    fprintf(af, " hostd==%s", mp->m_dk->d_host->h_hostname);
	  fprintf(af, ";fs:=%s", mp->m_name);

	  /*
	   * ... and a sublink if needed
	   */
	  if (mnlen < avlen) {
	    char *sublink = ap->a_volname + mnlen + 1;
	    fprintf(af, "/%s", sublink);
	  }
	  fputs(" ||", af);
	}
      }
    }

    /*
     * Next do the NFS locations
     */
    if (done_type_link)
      fputs(" -", af);

    ITER(dd, dict_data, &de->de_q) {
      fsi_mount *mp = (fsi_mount *) dd->dd_data;
      int namelen = mp->m_name_len;
      int exp_namelen = mp->m_exported->m_name_len;
      int volnlen = strlen(ap->a_volname);
      int mvolnlen = strlen(mp->m_volname);

      fputc(' ', af);

      /*
       * Output any selectors
       */
      if (mp->m_sel)
	fprintf(af, "%s;", mp->m_sel);

      /*
       * Print host and volname of exported filesystem
       */
      fprintf(af, "rhost:=%s",
	      mp->m_dk->d_host->h_lochost ?
	      mp->m_dk->d_host->h_lochost :
	      mp->m_dk->d_host->h_hostname);
      fprintf(af, ";rfs:=%s", mp->m_exported->m_volname);
      if (ap->a_opts && !STREQ(ap->a_opts, "")) {
	fprintf(af, ";%s", ap->a_opts);
      }

      /*
       * Now determine whether a sublink is required.
       */
      if (exp_namelen < namelen || mvolnlen < volnlen) {
	char sublink[1024];
	sublink[0] = '\0';
	if (exp_namelen < namelen) {
	  xstrlcat(sublink, mp->m_name + exp_namelen + 1, sizeof(sublink));
	  if (mvolnlen < volnlen)
	    xstrlcat(sublink, "/", sizeof(sublink));
	}
	if (mvolnlen < volnlen)
	  xstrlcat(sublink, ap->a_volname + mvolnlen + 1, sizeof(sublink));

	fprintf(af, ";sublink:=%s", sublink);
      }
    }
    fputc('\n', af);
  } else if (ap->a_symlink) {

    /*
     * A specific link.
     *
     * type:=link;fs:=whatever
     */
    fprintf(af, "%s type:=link;fs:=%s\n", ap->a_name + sk, ap->a_symlink);
  }

  return errors;
}


/*
 * Write a single automount configuration file
 */
static int
write_amount( qelem *q, char *def)
{
  automount *ap;
  int errors = 0;
  int direct = 0;

  /*
   * Output all indirect maps
   */
  ITER(ap, automount, q) {
    FILE *af;
    char *p;

    /*
     * If there is no a_mount node then this is really
     * a direct mount, so just keep a count and continue.
     * Direct mounts are output into a special file during
     * the second pass below.
     */
    if (!ap->a_mount) {
      direct++;
      continue;
    }

    p = strrchr(ap->a_name, '/');
    if (!p)
      p = ap->a_name;
    else
      p++;

    af = pref_open(mount_pref, p, gen_hdr, ap->a_name);
    if (af) {
      show_new(ap->a_name);
      fputs("/defaults ", af);
      if (*def)
	fprintf(af, "%s;", def);
      fputs("type:=nfs\n", af);
      errors += write_amount_info(af, ap, strlen(ap->a_name) + 1);
      errors += pref_close(af);
    }
  }

  /*
   * Output any direct map entries which were found during the
   * previous pass over the data.
   */
  if (direct) {
    FILE *af = pref_open(mount_pref, "direct.map", info_hdr, "direct mount");

    if (af) {
      show_new("direct mounts");
      fputs("/defaults ", af);
      if (*def)
	fprintf(af, "%s;", def);
      fputs("type:=nfs\n", af);
      ITER(ap, automount, q)
      if (!ap->a_mount)
	  errors += write_amount_info(af, ap, 1);
      errors += pref_close(af);
    }
  }
  return errors;
}


/*
 * Write all the needed automount configuration files
 */
int
write_atab(qelem *q)
{
  int errors = 0;

  if (mount_pref) {
    auto_tree *tp;
    show_area_being_processed("write automount", 5);
    ITER(tp, auto_tree, q)
      errors += write_amount(tp->t_mount, tp->t_defaults);
  }

  return errors;
}