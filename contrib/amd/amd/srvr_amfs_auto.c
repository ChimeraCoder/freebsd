
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
 * Automount FS server ("localhost") modeling
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#include <am_defs.h>
#include <amd.h>

/* globals */

/* statics */
static qelem amfs_auto_srvr_list = {&amfs_auto_srvr_list, &amfs_auto_srvr_list};
static fserver *localhost;


/*
 * Find an nfs server for the local host
 */
fserver *
amfs_generic_find_srvr(mntfs *mf)
{
  fserver *fs = localhost;

  if (!fs) {
    fs = ALLOC(struct fserver);
    fs->fs_refc = 0;
    fs->fs_host = strdup("localhost");
    fs->fs_ip = 0;
    fs->fs_cid = 0;
    fs->fs_pinger = AM_PINGER;
    fs->fs_flags = FSF_VALID | FSF_PING_UNINIT;
    fs->fs_type = "local";
    fs->fs_private = 0;
    fs->fs_prfree = 0;

    ins_que(&fs->fs_q, &amfs_auto_srvr_list);

    srvrlog(fs, "starts up");

    localhost = fs;
  }
  fs->fs_refc++;

  return fs;
}


/*****************************************************************************
 *** GENERIC ROUTINES FOLLOW
 *****************************************************************************/

/*
 * Wakeup anything waiting for this server
 */
void
wakeup_srvr(fserver *fs)
{
  fs->fs_flags &= ~FSF_WANT;
  wakeup((voidp) fs);
}


/*
 * Called when final ttl of server has expired
 */
static void
timeout_srvr(voidp v)
{
  fserver *fs = v;

  /*
   * If the reference count is still zero then
   * we are free to remove this node
   */
  if (fs->fs_refc == 0) {
    dlog("Deleting file server %s", fs->fs_host);
    if (fs->fs_flags & FSF_WANT)
      wakeup_srvr(fs);

    /*
     * Remove from queue.
     */
    rem_que(&fs->fs_q);
    /*
     * (Possibly) call the private free routine.
     */
    if (fs->fs_private && fs->fs_prfree)
      (*fs->fs_prfree) (fs->fs_private);

    /*
     * Free the net address
     */
    if (fs->fs_ip)
      XFREE(fs->fs_ip);

    /*
     * Free the host name.
     */
    XFREE(fs->fs_host);

    /*
     * Discard the fserver object.
     */
    XFREE(fs);
  }
}


/*
 * Free a file server
 */
void
free_srvr(fserver *fs)
{
  if (--fs->fs_refc == 0) {
    /*
     * The reference count is now zero,
     * so arrange for this node to be
     * removed in AM_TTL seconds if no
     * other mntfs is referencing it.
     */
    int ttl = (FSRV_ERROR(fs) || FSRV_ISDOWN(fs)) ? 19 : AM_TTL;

    dlog("Last hard reference to file server %s - will timeout in %ds", fs->fs_host, ttl);
    if (fs->fs_cid) {
      untimeout(fs->fs_cid);
      /*
       * Turn off pinging - XXX
       */
      fs->fs_flags &= ~FSF_PINGING;
    }

    /*
     * Keep structure lying around for a while
     */
    fs->fs_cid = timeout(ttl, timeout_srvr, (voidp) fs);

    /*
     * Mark the fileserver down and invalid again
     */
    fs->fs_flags &= ~FSF_VALID;
    fs->fs_flags |= FSF_DOWN;
  }
}


/*
 * Make a duplicate fserver reference
 */
fserver *
dup_srvr(fserver *fs)
{
  fs->fs_refc++;
  return fs;
}


/*
 * Log state change
 */
void
srvrlog(fserver *fs, char *state)
{
  plog(XLOG_INFO, "file server %s, type %s, state %s", fs->fs_host, fs->fs_type, state);
}