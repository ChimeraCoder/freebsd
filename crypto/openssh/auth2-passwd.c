
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

#include <string.h>
#include <stdarg.h>

#include "xmalloc.h"
#include "packet.h"
#include "log.h"
#include "key.h"
#include "hostfile.h"
#include "auth.h"
#include "buffer.h"
#ifdef GSSAPI
#include "ssh-gss.h"
#endif
#include "monitor_wrap.h"
#include "servconf.h"

/* import */
extern ServerOptions options;

static int
userauth_passwd(Authctxt *authctxt)
{
	char *password, *newpass;
	int authenticated = 0;
	int change;
	u_int len, newlen;

	change = packet_get_char();
	password = packet_get_string(&len);
	if (change) {
		/* discard new password from packet */
		newpass = packet_get_string(&newlen);
		memset(newpass, 0, newlen);
		xfree(newpass);
	}
	packet_check_eom();

	if (change)
		logit("password change not supported");
	else if (PRIVSEP(auth_password(authctxt, password)) == 1)
		authenticated = 1;
	memset(password, 0, len);
	xfree(password);
	return authenticated;
}

Authmethod method_passwd = {
	"password",
	userauth_passwd,
	&options.password_authentication
};