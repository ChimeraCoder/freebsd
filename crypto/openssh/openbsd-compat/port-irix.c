
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

#include "includes.h"

#if defined(WITH_IRIX_PROJECT) || \
    defined(WITH_IRIX_JOBS) || \
    defined(WITH_IRIX_ARRAY)

#include <errno.h>
#include <string.h>
#include <unistd.h>

#ifdef WITH_IRIX_PROJECT
# include <proj.h>
#endif /* WITH_IRIX_PROJECT */
#ifdef WITH_IRIX_JOBS
# include <sys/resource.h>
#endif
#ifdef WITH_IRIX_AUDIT
# include <sat.h>
#endif /* WITH_IRIX_AUDIT */

void
irix_setusercontext(struct passwd *pw)
{
#ifdef WITH_IRIX_PROJECT
        prid_t projid;
#endif
#ifdef WITH_IRIX_JOBS
        jid_t jid = 0;
#elif defined(WITH_IRIX_ARRAY)
        int jid = 0;
#endif

#ifdef WITH_IRIX_JOBS
        jid = jlimit_startjob(pw->pw_name, pw->pw_uid, "interactive");
        if (jid == -1)
                fatal("Failed to create job container: %.100s",
                    strerror(errno));
#endif /* WITH_IRIX_JOBS */
#ifdef WITH_IRIX_ARRAY
        /* initialize array session */
        if (jid == 0  && newarraysess() != 0)
                fatal("Failed to set up new array session: %.100s",
                    strerror(errno));
#endif /* WITH_IRIX_ARRAY */
#ifdef WITH_IRIX_PROJECT
        /* initialize irix project info */
        if ((projid = getdfltprojuser(pw->pw_name)) == -1) {
                debug("Failed to get project id, using projid 0");
                projid = 0;
        }
        if (setprid(projid))
                fatal("Failed to initialize project %d for %s: %.100s",
                    (int)projid, pw->pw_name, strerror(errno));
#endif /* WITH_IRIX_PROJECT */
#ifdef WITH_IRIX_AUDIT
        if (sysconf(_SC_AUDIT)) {
                debug("Setting sat id to %d", (int) pw->pw_uid);
                if (satsetid(pw->pw_uid))
                        debug("error setting satid: %.100s", strerror(errno));
        }
#endif /* WITH_IRIX_AUDIT */
}


#endif /* defined(WITH_IRIX_PROJECT) || defined(WITH_IRIX_JOBS) || defined(WITH_IRIX_ARRAY) */