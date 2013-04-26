
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
write_export_info(FILE *ef, qelem *q, int errors)
{
  fsi_mount *mp;

  ITER(mp, fsi_mount, q) {
    if (mp->m_mask & (1 << DM_EXPORTFS))
      fprintf(ef, "%s\t%s\n", mp->m_volname, mp->m_exportfs);
    if (mp->m_mount)
      errors += write_export_info(ef, mp->m_mount, 0);
  }

  return errors;
}


static int
write_dkexports(FILE *ef, qelem *q)
{
  int errors = 0;
  disk_fs *dp;

  ITER(dp, disk_fs, q) {
    if (dp->d_mount)
      errors += write_export_info(ef, dp->d_mount, 0);
  }

  return errors;
}


int
write_exportfs(qelem *q)
{
  int errors = 0;

  if (exportfs_pref) {
    host *hp;

    show_area_being_processed("write exportfs", 5);
    ITER(hp, host, q) {
      if (hp->h_disk_fs) {
	FILE *ef = pref_open(exportfs_pref, hp->h_hostname, gen_hdr, hp->h_hostname);
	if (ef) {
	  show_new(hp->h_hostname);
	  errors += write_dkexports(ef, hp->h_disk_fs);
	  errors += pref_close(ef);
	} else {
	  errors++;
	}
      }
    }
  }

  return errors;
}