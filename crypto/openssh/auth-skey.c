
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

#ifdef SKEY

#include <sys/types.h>

#include <pwd.h>
#include <stdio.h>

#include <skey.h>

#include "xmalloc.h"
#include "key.h"
#include "hostfile.h"
#include "auth.h"
#include "ssh-gss.h"
#include "log.h"
#include "monitor_wrap.h"

static void *
skey_init_ctx(Authctxt *authctxt)
{
	return authctxt;
}

int
skey_query(void *ctx, char **name, char **infotxt,
    u_int* numprompts, char ***prompts, u_int **echo_on)
{
	Authctxt *authctxt = ctx;
	char challenge[1024];
	struct skey skey;

	if (_compat_skeychallenge(&skey, authctxt->user, challenge,
	    sizeof(challenge)) == -1)
		return -1;

	*name = xstrdup("");
	*infotxt = xstrdup("");
	*numprompts = 1;
	*prompts = xcalloc(*numprompts, sizeof(char *));
	*echo_on = xcalloc(*numprompts, sizeof(u_int));

	xasprintf(*prompts, "%s%s", challenge, SKEY_PROMPT);

	return 0;
}

int
skey_respond(void *ctx, u_int numresponses, char **responses)
{
	Authctxt *authctxt = ctx;

	if (authctxt->valid &&
	    numresponses == 1 &&
	    skey_haskey(authctxt->pw->pw_name) == 0 &&
	    skey_passcheck(authctxt->pw->pw_name, responses[0]) != -1)
	    return 0;
	return -1;
}

static void
skey_free_ctx(void *ctx)
{
	/* we don't have a special context */
}

KbdintDevice skey_device = {
	"skey",
	skey_init_ctx,
	skey_query,
	skey_respond,
	skey_free_ctx
};

KbdintDevice mm_skey_device = {
	"skey",
	skey_init_ctx,
	mm_skey_query,
	mm_skey_respond,
	skey_free_ctx
};
#endif /* SKEY */