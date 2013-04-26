
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
#include <sys/stat.h>
#include <sys/uio.h>

#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "atomicio.h"
#include "xmalloc.h"
#include "key.h"
#include "hostfile.h"
#include "auth.h"
#include "packet.h"
#include "log.h"
#include "buffer.h"
#include "servconf.h"
#include "compat.h"
#include "ssh2.h"
#ifdef GSSAPI
#include "ssh-gss.h"
#endif
#include "monitor_wrap.h"

/* import */
extern ServerOptions options;

/* "none" is allowed only one time */
static int none_enabled = 1;

static int
userauth_none(Authctxt *authctxt)
{
	none_enabled = 0;
	packet_check_eom();
	if (options.permit_empty_passwd && options.password_authentication)
		return (PRIVSEP(auth_password(authctxt, "")));
	return (0);
}

Authmethod method_none = {
	"none",
	userauth_none,
	&none_enabled
};