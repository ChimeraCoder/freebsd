
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

#if defined(USE_SHADOW) && defined(HAS_SHADOW_EXPIRE)
#include <shadow.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "key.h"
#include "hostfile.h"
#include "auth.h"
#include "buffer.h"
#include "log.h"

#ifdef DAY
# undef DAY
#endif
#define DAY	(24L * 60 * 60) /* 1 day in seconds */

extern Buffer loginmsg;

/*
 * For the account and password expiration functions, we assume the expiry
 * occurs the day after the day specified.
 */

/*
 * Check if specified account is expired.  Returns 1 if account is expired,
 * 0 otherwise.
 */
int
auth_shadow_acctexpired(struct spwd *spw)
{
	time_t today;
	int daysleft;
	char buf[256];

	today = time(NULL) / DAY;
	daysleft = spw->sp_expire - today;
	debug3("%s: today %d sp_expire %d days left %d", __func__, (int)today,
	    (int)spw->sp_expire, daysleft);

	if (spw->sp_expire == -1) {
		debug3("account expiration disabled");
	} else if (daysleft < 0) {
		logit("Account %.100s has expired", spw->sp_namp);
		return 1;
	} else if (daysleft <= spw->sp_warn) {
		debug3("account will expire in %d days", daysleft);
		snprintf(buf, sizeof(buf),
		    "Your account will expire in %d day%s.\n", daysleft,
		    daysleft == 1 ? "" : "s");
		buffer_append(&loginmsg, buf, strlen(buf));
	}

	return 0;
}

/*
 * Checks password expiry for platforms that use shadow passwd files.
 * Returns: 1 = password expired, 0 = password not expired
 */
int
auth_shadow_pwexpired(Authctxt *ctxt)
{
	struct spwd *spw = NULL;
	const char *user = ctxt->pw->pw_name;
	char buf[256];
	time_t today;
	int daysleft, disabled = 0;

	if ((spw = getspnam((char *)user)) == NULL) {
		error("Could not get shadow information for %.100s", user);
		return 0;
	}

	today = time(NULL) / DAY;
	debug3("%s: today %d sp_lstchg %d sp_max %d", __func__, (int)today,
	    (int)spw->sp_lstchg, (int)spw->sp_max);

#if defined(__hpux) && !defined(HAVE_SECUREWARE)
	if (iscomsec()) {
		struct pr_passwd *pr;

		pr = getprpwnam((char *)user);

		/* Test for Trusted Mode expiry disabled */
		if (pr != NULL && pr->ufld.fd_min == 0 &&
		    pr->ufld.fd_lifetime == 0 && pr->ufld.fd_expire == 0 &&
		    pr->ufld.fd_pw_expire_warning == 0 &&
		    pr->ufld.fd_schange != 0)
			disabled = 1;
	}
#endif

	/* TODO: check sp_inact */
	daysleft = spw->sp_lstchg + spw->sp_max - today;
	if (disabled) {
		debug3("password expiration disabled");
	} else if (spw->sp_lstchg == 0) {
		logit("User %.100s password has expired (root forced)", user);
		return 1;
	} else if (spw->sp_max == -1) {
		debug3("password expiration disabled");
	} else if (daysleft < 0) {
		logit("User %.100s password has expired (password aged)", user);
		return 1;
	} else if (daysleft <= spw->sp_warn) {
		debug3("password will expire in %d days", daysleft);
		snprintf(buf, sizeof(buf),
		    "Your password will expire in %d day%s.\n", daysleft,
		    daysleft == 1 ? "" : "s");
		buffer_append(&loginmsg, buf, strlen(buf));
	}

	return 0;
}
#endif	/* USE_SHADOW && HAS_SHADOW_EXPIRE */