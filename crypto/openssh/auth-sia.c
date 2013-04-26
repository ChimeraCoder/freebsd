
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

#ifdef HAVE_OSF_SIA
#include <sia.h>
#include <siad.h>
#include <pwd.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#include "ssh.h"
#include "key.h"
#include "hostfile.h"
#include "auth.h"
#include "auth-sia.h"
#include "log.h"
#include "servconf.h"
#include "canohost.h"
#include "uidswap.h"

extern ServerOptions options;
extern int saved_argc;
extern char **saved_argv;

int
sys_auth_passwd(Authctxt *authctxt, const char *pass)
{
	int ret;
	SIAENTITY *ent = NULL;
	const char *host;

	host = get_canonical_hostname(options.use_dns);

	if (!authctxt->user || pass == NULL || pass[0] == '\0')
		return (0);

	if (sia_ses_init(&ent, saved_argc, saved_argv, host, authctxt->user,
	    NULL, 0, NULL) != SIASUCCESS)
		return (0);

	if ((ret = sia_ses_authent(NULL, pass, ent)) != SIASUCCESS) {
		error("Couldn't authenticate %s from %s",
		    authctxt->user, host);
		if (ret & SIASTOP)
			sia_ses_release(&ent);

		return (0);
	}

	sia_ses_release(&ent);

	return (1);
}

void
session_setup_sia(struct passwd *pw, char *tty)
{
	SIAENTITY *ent = NULL;
	const char *host;

	host = get_canonical_hostname(options.use_dns);

	if (sia_ses_init(&ent, saved_argc, saved_argv, host, pw->pw_name,
	    tty, 0, NULL) != SIASUCCESS)
		fatal("sia_ses_init failed");

	if (sia_make_entity_pwd(pw, ent) != SIASUCCESS) {
		sia_ses_release(&ent);
		fatal("sia_make_entity_pwd failed");
	}

	ent->authtype = SIA_A_NONE;
	if (sia_ses_estab(sia_collect_trm, ent) != SIASUCCESS)
		fatal("Couldn't establish session for %s from %s",
		    pw->pw_name, host);

	if (sia_ses_launch(sia_collect_trm, ent) != SIASUCCESS)
		fatal("Couldn't launch session for %s from %s",
		    pw->pw_name, host);

	sia_ses_release(&ent);

	setuid(0);
	permanently_set_uid(pw);
}

#endif /* HAVE_OSF_SIA */