
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

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "loginrec.h"
#include "log.h"
#include "buffer.h"
#include "servconf.h"

extern Buffer loginmsg;
extern ServerOptions options;

/*
 * Returns the time when the user last logged in.  Returns 0 if the
 * information is not available.  This must be called before record_login.
 * The host the user logged in from will be returned in buf.
 */
time_t
get_last_login_time(uid_t uid, const char *logname,
    char *buf, size_t bufsize)
{
	struct logininfo li;

	login_get_lastlog(&li, uid);
	strlcpy(buf, li.hostname, bufsize);
	return (time_t)li.tv_sec;
}

/*
 * Generate and store last login message.  This must be done before
 * login_login() is called and lastlog is updated.
 */
static void
store_lastlog_message(const char *user, uid_t uid)
{
#ifndef NO_SSH_LASTLOG
	char *time_string, hostname[MAXHOSTNAMELEN] = "", buf[512];
	time_t last_login_time;

	if (!options.print_lastlog)
		return;

# ifdef CUSTOM_SYS_AUTH_GET_LASTLOGIN_MSG
	time_string = sys_auth_get_lastlogin_msg(user, uid);
	if (time_string != NULL) {
		buffer_append(&loginmsg, time_string, strlen(time_string));
		xfree(time_string);
	}
# else
	last_login_time = get_last_login_time(uid, user, hostname,
	    sizeof(hostname));

	if (last_login_time != 0) {
		time_string = ctime(&last_login_time);
		time_string[strcspn(time_string, "\n")] = '\0';
		if (strcmp(hostname, "") == 0)
			snprintf(buf, sizeof(buf), "Last login: %s\r\n",
			    time_string);
		else
			snprintf(buf, sizeof(buf), "Last login: %s from %s\r\n",
			    time_string, hostname);
		buffer_append(&loginmsg, buf, strlen(buf));
	}
# endif /* CUSTOM_SYS_AUTH_GET_LASTLOGIN_MSG */
#endif /* NO_SSH_LASTLOG */
}

/*
 * Records that the user has logged in.  I wish these parts of operating
 * systems were more standardized.
 */
void
record_login(pid_t pid, const char *tty, const char *user, uid_t uid,
    const char *host, struct sockaddr *addr, socklen_t addrlen)
{
	struct logininfo *li;

	/* save previous login details before writing new */
	store_lastlog_message(user, uid);

	li = login_alloc_entry(pid, user, host, tty);
	login_set_addr(li, addr, addrlen);
	login_login(li);
	login_free_entry(li);
}

#ifdef LOGIN_NEEDS_UTMPX
void
record_utmp_only(pid_t pid, const char *ttyname, const char *user,
		 const char *host, struct sockaddr *addr, socklen_t addrlen)
{
	struct logininfo *li;

	li = login_alloc_entry(pid, user, host, ttyname);
	login_set_addr(li, addr, addrlen);
	login_utmp_only(li);
	login_free_entry(li);
}
#endif

/* Records that the user has logged out. */
void
record_logout(pid_t pid, const char *tty, const char *user)
{
	struct logininfo *li;

	li = login_alloc_entry(pid, user, NULL, tty);
	login_logout(li);
	login_free_entry(li);
}