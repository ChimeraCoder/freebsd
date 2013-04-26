
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
#include <sys/param.h>
#include <sys/socket.h>

#include <ctype.h>
#include <netdb.h>
#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <cflib.h>

#include <netsmb/netbios.h>
#include <netsmb/smb_lib.h>
#include <netsmb/nb_lib.h>

int
nb_ctx_create(struct nb_ctx **ctxpp)
{
	struct nb_ctx *ctx;

	ctx = malloc(sizeof(struct nb_ctx));
	if (ctx == NULL)
		return ENOMEM;
	bzero(ctx, sizeof(struct nb_ctx));
	ctx->nb_nmbtcpport = NMB_TCP_PORT;
	ctx->nb_smbtcpport = SMB_TCP_PORT;

	*ctxpp = ctx;
	return 0;
}

void
nb_ctx_done(struct nb_ctx *ctx)
{
	if (ctx == NULL)
		return;
	if (ctx->nb_scope)
		free(ctx->nb_scope);
}

int
nb_ctx_setns(struct nb_ctx *ctx, const char *addr)
{
	if (addr == NULL || addr[0] == 0)
		return EINVAL;
	if (ctx->nb_nsname)
		free(ctx->nb_nsname);
	if ((ctx->nb_nsname = strdup(addr)) == NULL)
		return ENOMEM;
	return 0;
}

int
nb_ctx_setscope(struct nb_ctx *ctx, const char *scope)
{
	size_t slen = strlen(scope);

	if (slen >= 128) {
		smb_error("scope '%s' is too long", 0, scope);
		return ENAMETOOLONG;
	}
	if (ctx->nb_scope)
		free(ctx->nb_scope);
	ctx->nb_scope = malloc(slen + 1);
	if (ctx->nb_scope == NULL)
		return ENOMEM;
	nls_str_upper(ctx->nb_scope, scope);
	return 0;
}

int
nb_ctx_resolve(struct nb_ctx *ctx)
{
	struct sockaddr *sap;
	int error;

	ctx->nb_flags &= ~NBCF_RESOLVED;

	if (ctx->nb_nsname == NULL) {
		ctx->nb_ns.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	} else {
		error = nb_resolvehost_in(ctx->nb_nsname, &sap, ctx->nb_smbtcpport);
		if (error) {
			smb_error("can't resolve %s", error, ctx->nb_nsname);
			return error;
		}
		if (sap->sa_family != AF_INET) {
			smb_error("unsupported address family %d", 0, sap->sa_family);
			return EINVAL;
		}
		bcopy(sap, &ctx->nb_ns, sizeof(ctx->nb_ns));
		free(sap);
	}
	ctx->nb_ns.sin_port = htons(ctx->nb_nmbtcpport);
	ctx->nb_ns.sin_family = AF_INET;
	ctx->nb_ns.sin_len = sizeof(ctx->nb_ns);
	ctx->nb_flags |= NBCF_RESOLVED;
	return 0;
}

/*
 * used level values:
 * 0 - default
 * 1 - server
 */
int
nb_ctx_readrcsection(struct rcfile *rcfile, struct nb_ctx *ctx,
	const char *sname, int level)
{
	char *p;
	int error;

	if (level > 1)
		return EINVAL;
	rc_getint(rcfile, sname, "nbtimeout", &ctx->nb_timo);
	rc_getstringptr(rcfile, sname, "nbns", &p);
	if (p) {
		error = nb_ctx_setns(ctx, p);
		if (error) {
			smb_error("invalid address specified in the section %s", 0, sname);
			return error;
		}
	}
	rc_getstringptr(rcfile, sname, "nbscope", &p);
	if (p)
		nb_ctx_setscope(ctx, p);
	return 0;
}

static const char *nb_err_rcode[] = {
	"bad request/response format",
	"NBNS server failure",
	"no such name",
	"unsupported request",
	"request rejected",
	"name already registered"
};

static const char *nb_err[] = {
	"host not found",
	"too many redirects",
	"invalid response",
	"NETBIOS name too long",
	"no interface to broadcast on and no NBNS server specified"
};

const char *
nb_strerror(int error)
{
	if (error == 0)
		return NULL;
	if (error <= NBERR_ACTIVE)
		return nb_err_rcode[error - 1];
	else if (error >= NBERR_HOSTNOTFOUND && error < NBERR_MAX)
		return nb_err[error - NBERR_HOSTNOTFOUND];
	else
		return NULL;
}