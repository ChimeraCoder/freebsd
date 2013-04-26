
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

#include "login_locl.h"
RCSID("$Id$");

int
do_osfc2_magic(uid_t uid)
{
#ifdef HAVE_OSFC2
    struct es_passwd *epw;
    char *argv[2];

    /* fake */
    argv[0] = (char*)getprogname();
    argv[1] = NULL;
    set_auth_parameters(1, argv);

    epw = getespwuid(uid);
    if(epw == NULL) {
	syslog(LOG_AUTHPRIV|LOG_NOTICE,
	       "getespwuid failed for %d", uid);
	printf("Sorry.\n");
	return 1;
    }
    /* We don't check for auto-retired, foo-retired,
       bar-retired, or any other kind of retired accounts
       here; neither do we check for time-locked accounts, or
       any other kind of serious C2 mumbo-jumbo. We do,
       however, call setluid, since failing to do so is not
       very good (take my word for it). */

    if(!epw->uflg->fg_uid) {
	syslog(LOG_AUTHPRIV|LOG_NOTICE,
	       "attempted login by %s (has no uid)", epw->ufld->fd_name);
	printf("Sorry.\n");
	return 1;
    }
    setluid(epw->ufld->fd_uid);
    if(getluid() != epw->ufld->fd_uid) {
	syslog(LOG_AUTHPRIV|LOG_NOTICE,
	       "failed to set LUID for %s (%d)",
	       epw->ufld->fd_name, epw->ufld->fd_uid);
	printf("Sorry.\n");
	return 1;
    }
#endif /* HAVE_OSFC2 */
    return 0;
}