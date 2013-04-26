
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

#include <stdarg.h>

#include "xmalloc.h"
#include "packet.h"
#include "key.h"
#include "hostfile.h"
#include "auth.h"
#include "log.h"
#include "buffer.h"
#include "servconf.h"

/* import */
extern ServerOptions options;

static int
userauth_kbdint(Authctxt *authctxt)
{
	int authenticated = 0;
	char *lang, *devs;

	lang = packet_get_string(NULL);
	devs = packet_get_string(NULL);
	packet_check_eom();

	debug("keyboard-interactive devs %s", devs);

	if (options.challenge_response_authentication)
		authenticated = auth2_challenge(authctxt, devs);

	xfree(devs);
	xfree(lang);
	return authenticated;
}

Authmethod method_kbdint = {
	"keyboard-interactive",
	userauth_kbdint,
	&options.kbd_interactive_authentication
};