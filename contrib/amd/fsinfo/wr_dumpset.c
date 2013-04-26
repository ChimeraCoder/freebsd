
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


static int
write_dumpset_info(FILE *ef, qelem *q)
{
  int errors = 0;
  disk_fs *dp;

  ITER(dp, disk_fs, q) {
    if (dp->d_dumpset) {
      fprintf(ef, "%s\t%s:%-30s\t# %s\n",
	      dp->d_dumpset,
	      dp->d_host->h_lochost ?
	      dp->d_host->h_lochost :
	      dp->d_host->h_hostname,
	      dp->d_mountpt,
	      dp->d_dev);
    }
  }
  return errors;
}


int
write_dumpset(qelem *q)
{
  int errors = 0;

  if (dumpset_pref) {
    FILE *ef = pref_open(dumpset_pref, "dumpsets", info_hdr, "exabyte dumpset");
    if (ef) {
      host *hp;

      ITER(hp, host, q) {
	if (hp->h_disk_fs) {
	  errors += write_dumpset_info(ef, hp->h_disk_fs);
	}
      }
      errors += pref_close(ef);
    } else {
      errors++;
    }
  }

  return errors;
}