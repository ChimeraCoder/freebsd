
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
#include "key.h"
#include "hostfile.h"
#include "auth.h"
#include "log.h"
#include "servconf.h"

/* limited protocol v1 interface to kbd-interactive authentication */

extern KbdintDevice *devices[];
static KbdintDevice *device;
extern ServerOptions options;

char *
get_challenge(Authctxt *authctxt)
{
	char *challenge, *name, *info, **prompts;
	u_int i, numprompts;
	u_int *echo_on;

#ifdef USE_PAM
	if (!options.use_pam)
		remove_kbdint_device("pam");
#endif

	device = devices[0]; /* we always use the 1st device for protocol 1 */
	if (device == NULL)
		return NULL;
	if ((authctxt->kbdintctxt = device->init_ctx(authctxt)) == NULL)
		return NULL;
	if (device->query(authctxt->kbdintctxt, &name, &info,
	    &numprompts, &prompts, &echo_on)) {
		device->free_ctx(authctxt->kbdintctxt);
		authctxt->kbdintctxt = NULL;
		return NULL;
	}
	if (numprompts < 1)
		fatal("get_challenge: numprompts < 1");
	challenge = xstrdup(prompts[0]);
	for (i = 0; i < numprompts; i++)
		xfree(prompts[i]);
	xfree(prompts);
	xfree(name);
	xfree(echo_on);
	xfree(info);

	return (challenge);
}
int
verify_response(Authctxt *authctxt, const char *response)
{
	char *resp[1], *name, *info, **prompts;
	u_int i, numprompts, *echo_on;
	int authenticated = 0;

	if (device == NULL)
		return 0;
	if (authctxt->kbdintctxt == NULL)
		return 0;
	resp[0] = (char *)response;
	switch (device->respond(authctxt->kbdintctxt, 1, resp)) {
	case 0: /* Success */
		authenticated = 1;
		break;
	case 1: /* Postponed - retry with empty query for PAM */
		if ((device->query(authctxt->kbdintctxt, &name, &info,
		    &numprompts, &prompts, &echo_on)) != 0)
			break;
		if (numprompts == 0 &&
		    device->respond(authctxt->kbdintctxt, 0, resp) == 0)
			authenticated = 1;

		for (i = 0; i < numprompts; i++)
			xfree(prompts[i]);
		xfree(prompts);
		xfree(name);
		xfree(echo_on);
		xfree(info);
		break;
	}
	device->free_ctx(authctxt->kbdintctxt);
	authctxt->kbdintctxt = NULL;
	return authenticated;
}
void
abandon_challenge_response(Authctxt *authctxt)
{
	if (authctxt->kbdintctxt != NULL) {
		device->free_ctx(authctxt->kbdintctxt);
		authctxt->kbdintctxt = NULL;
	}
}