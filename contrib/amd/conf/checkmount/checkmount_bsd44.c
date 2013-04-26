
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

extern int is_same_host(char *name1, char *name2, struct in_addr addr2);
int fixmount_check_mount(char *host, struct in_addr hostaddr, char *path);

int
fixmount_check_mount(char *host, struct in_addr hostaddr, char *path)
{
  struct statfs *mntbufp, *mntp;
  int nloc, i;
  char *colon;

  /* read mount table from kernel */
  nloc = getmntinfo(&mntbufp, MNT_NOWAIT);
  if (nloc <= 0) {
    perror("getmntinfo");
    exit(1);
  }

  mntp = mntbufp;
  for (i=0; i<nloc; ++i) {
    if ((colon = strchr(mntp->f_mntfromname, ':'))) {
      *colon = '\0';
      if (STREQ(colon + 1, path) &&
	  is_same_host(mntp->f_mntfromname, host, hostaddr))
	return 1;
    }
  }

  return 0;
}