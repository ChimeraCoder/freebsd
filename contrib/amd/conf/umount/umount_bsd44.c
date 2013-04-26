
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
 * Unmounting filesystems under BSD 4.4.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amu.h>


int
umount_fs(char *mntdir, const char *mnttabname, u_int unmount_flags)
{
  int error;

eintr:
  error = unmount(mntdir, 0);
  if (error < 0)
    error = errno;

  switch (error) {
  case EINVAL:
  case ENOTBLK:
  case ENOENT:
    plog(XLOG_WARNING, "unmount: %s is not mounted", mntdir);
    error = 0;			/* Not really an error */
    break;

  case EINTR:
    /* not sure why this happens, but it does.  ask kirk one day... */
    dlog("%s: unmount: %m", mntdir);
    goto eintr;

#ifdef MNT2_GEN_OPT_FORCE
  case EBUSY:
  case EIO:
  case ESTALE:
    /* caller determines if forced unmounts should be used */
    if (unmount_flags & AMU_UMOUNT_FORCE) {
      error = umount2_fs(mntdir, unmount_flags);
      if (error < 0)
	error = errno;
      else
	return error;
    }
    /* fallthrough */
#endif /* MNT2_GEN_OPT_FORCE */

  default:
    dlog("%s: unmount: %m", mntdir);
    break;
  }

  return error;
}


#ifdef MNT2_GEN_OPT_FORCE
/* force unmount, no questions asked, without touching mnttab file */
int
umount2_fs(const char *mntdir, u_int unmount_flags)
{
  int error = 0;
  if (unmount_flags & AMU_UMOUNT_FORCE) {
    plog(XLOG_INFO, "umount2_fs: trying unmount/forced on %s", mntdir);
    error = unmount(mntdir, MNT2_GEN_OPT_FORCE);
    if (error < 0 && (errno == EINVAL || errno == ENOENT))
      error = 0;		/* ignore EINVAL/ENOENT */
    if (error < 0)
      plog(XLOG_WARNING, "%s: unmount/force: %m", mntdir);
    else
      dlog("%s: unmount/force: OK", mntdir);
  }
  return error;
}
#endif /* MNT2_GEN_OPT_FORCE */