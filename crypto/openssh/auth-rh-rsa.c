
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

#include <pwd.h>
#include <stdarg.h>

#include "packet.h"
#include "uidswap.h"
#include "log.h"
#include "buffer.h"
#include "servconf.h"
#include "key.h"
#include "hostfile.h"
#include "pathnames.h"
#include "auth.h"
#include "canohost.h"
#ifdef GSSAPI
#include "ssh-gss.h"
#endif
#include "monitor_wrap.h"

/* import */
extern ServerOptions options;

int
auth_rhosts_rsa_key_allowed(struct passwd *pw, char *cuser, char *chost,
    Key *client_host_key)
{
	HostStatus host_status;

	if (auth_key_is_revoked(client_host_key))
		return 0;

	/* Check if we would accept it using rhosts authentication. */
	if (!auth_rhosts(pw, cuser))
		return 0;

	host_status = check_key_in_hostfiles(pw, client_host_key,
	    chost, _PATH_SSH_SYSTEM_HOSTFILE,
	    options.ignore_user_known_hosts ? NULL : _PATH_SSH_USER_HOSTFILE);

	return (host_status == HOST_OK);
}

/*
 * Tries to authenticate the user using the .rhosts file and the host using
 * its host key.  Returns true if authentication succeeds.
 */
int
auth_rhosts_rsa(Authctxt *authctxt, char *cuser, Key *client_host_key)
{
	char *chost;
	struct passwd *pw = authctxt->pw;

	debug("Trying rhosts with RSA host authentication for client user %.100s",
	    cuser);

	if (!authctxt->valid || client_host_key == NULL ||
	    client_host_key->rsa == NULL)
		return 0;

	chost = (char *)get_canonical_hostname(options.use_dns);
	debug("Rhosts RSA authentication: canonical host %.900s", chost);

	if (!PRIVSEP(auth_rhosts_rsa_key_allowed(pw, cuser, chost, client_host_key))) {
		debug("Rhosts with RSA host authentication denied: unknown or invalid host key");
		packet_send_debug("Your host key cannot be verified: unknown or invalid host key.");
		return 0;
	}
	/* A matching host key was found and is known. */

	/* Perform the challenge-response dialog with the client for the host key. */
	if (!auth_rsa_challenge_dialog(client_host_key)) {
		logit("Client on %.800s failed to respond correctly to host authentication.",
		    chost);
		return 0;
	}
	/*
	 * We have authenticated the user using .rhosts or /etc/hosts.equiv,
	 * and the host using RSA. We accept the authentication.
	 */

	verbose("Rhosts with RSA host authentication accepted for %.100s, %.100s on %.700s.",
	    pw->pw_name, cuser, chost);
	packet_send_debug("Rhosts with RSA host authentication accepted.");
	return 1;
}