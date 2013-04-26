
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

#ifdef BSD_AUTH
#include "xmalloc.h"
#include "key.h"
#include "hostfile.h"
#include "auth.h"
#include "log.h"
#include "buffer.h"
#ifdef GSSAPI
#include "ssh-gss.h"
#endif
#include "monitor_wrap.h"

static void *
bsdauth_init_ctx(Authctxt *authctxt)
{
	return authctxt;
}

int
bsdauth_query(void *ctx, char **name, char **infotxt,
   u_int *numprompts, char ***prompts, u_int **echo_on)
{
	Authctxt *authctxt = ctx;
	char *challenge = NULL;

	if (authctxt->as != NULL) {
		debug2("bsdauth_query: try reuse session");
		challenge = auth_getitem(authctxt->as, AUTHV_CHALLENGE);
		if (challenge == NULL) {
			auth_close(authctxt->as);
			authctxt->as = NULL;
		}
	}

	if (challenge == NULL) {
		debug2("bsdauth_query: new bsd auth session");
		debug3("bsdauth_query: style %s",
		    authctxt->style ? authctxt->style : "<default>");
		authctxt->as = auth_userchallenge(authctxt->user,
		    authctxt->style, "auth-ssh", &challenge);
		if (authctxt->as == NULL)
			challenge = NULL;
		debug2("bsdauth_query: <%s>", challenge ? challenge : "empty");
	}

	if (challenge == NULL)
		return -1;

	*name = xstrdup("");
	*infotxt = xstrdup("");
	*numprompts = 1;
	*prompts = xcalloc(*numprompts, sizeof(char *));
	*echo_on = xcalloc(*numprompts, sizeof(u_int));
	(*prompts)[0] = xstrdup(challenge);

	return 0;
}

int
bsdauth_respond(void *ctx, u_int numresponses, char **responses)
{
	Authctxt *authctxt = ctx;
	int authok;

	if (!authctxt->valid)
		return -1;

	if (authctxt->as == 0)
		error("bsdauth_respond: no bsd auth session");

	if (numresponses != 1)
		return -1;

	authok = auth_userresponse(authctxt->as, responses[0], 0);
	authctxt->as = NULL;
	debug3("bsdauth_respond: <%s> = <%d>", responses[0], authok);

	return (authok == 0) ? -1 : 0;
}

static void
bsdauth_free_ctx(void *ctx)
{
	Authctxt *authctxt = ctx;

	if (authctxt && authctxt->as) {
		auth_close(authctxt->as);
		authctxt->as = NULL;
	}
}

KbdintDevice bsdauth_device = {
	"bsdauth",
	bsdauth_init_ctx,
	bsdauth_query,
	bsdauth_respond,
	bsdauth_free_ctx
};

KbdintDevice mm_bsdauth_device = {
	"bsdauth",
	bsdauth_init_ctx,
	mm_bsdauth_query,
	mm_bsdauth_respond,
	bsdauth_free_ctx
};
#endif