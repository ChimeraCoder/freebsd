
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
 * Write a host/path in NFS format
 */
static int
write_nfsname(FILE *ef, fsmount *fp, char *hn)
{
  int errors = 0;
  char *h = strdup(fp->f_ref->m_dk->d_host->h_hostname);

 domain_strip(h, hn);
  fprintf(ef, "%s:%s", h, fp->f_volname);
  XFREE(h);
  return errors;
}


/*
 * Write a bootparams entry for a host
 */
static int
write_boot_info(FILE *ef, host *hp)
{
  int errors = 0;

  fprintf(ef, "%s\troot=", hp->h_hostname);
  errors += write_nfsname(ef, hp->h_netroot, hp->h_hostname);
  fputs(" swap=", ef);
  errors += write_nfsname(ef, hp->h_netswap, hp->h_hostname);
  fputs("\n", ef);

  return 0;
}


/*
 * Output a bootparams file
 */
int
write_bootparams(qelem *q)
{
  int errors = 0;

  if (bootparams_pref) {
    FILE *ef = pref_open(bootparams_pref, "bootparams", info_hdr, "bootparams");
    if (ef) {
      host *hp;
      ITER(hp, host, q)
      if (hp->h_netroot && hp->h_netswap)
	  errors += write_boot_info(ef, hp);
      errors += pref_close(ef);
    } else {
      errors++;
    }
  }

  return errors;
}