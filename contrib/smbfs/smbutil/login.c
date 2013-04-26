
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
#include <sys/errno.h>
#include <sys/stat.h>
#include <err.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <sysexits.h>

#include <cflib.h>

#include <netsmb/smb_lib.h>
#include <netsmb/smb_conn.h>

#include "common.h"


int
cmd_login(int argc, char *argv[])
{
	struct smb_ctx sctx, *ctx = &sctx;
	int error, opt, setprimary = 0, level;

	if (argc < 2)
		login_usage();
	if (smb_ctx_init(ctx, argc, argv, SMBL_VC, SMBL_SHARE, SMB_ST_ANY) != 0)
		exit(1);
	if (smb_ctx_readrc(ctx) != 0)
		exit(1);
	if (smb_rc)
		rc_close(smb_rc);
	while ((opt = getopt(argc, argv, STDPARAM_OPT"D")) != EOF) {
		switch(opt){
		    case STDPARAM_ARGS:
			error = smb_ctx_opt(ctx, opt, optarg);
			if (error)
				exit(1);
			break;
		    case 'D':
			setprimary = 1;
			break;
		    default:
			login_usage();
			/*NOTREACHED*/
		}
	}
#ifdef APPLE
	if (loadsmbvfs())
		errx(EX_OSERR, "SMB filesystem is not available");
#endif
	if (smb_ctx_resolve(ctx) != 0)
		exit(1);
	level = ctx->ct_parsedlevel;
	error = smb_ctx_lookup(ctx, level, 0);
	if (error == 0) {
		smb_error("connection already exists", error);
		exit(0);
	}
	error = smb_ctx_lookup(ctx, level, SMBLK_CREATE);
	if (error) {
		smb_error("could not login to server %s", error, ctx->ct_ssn.ioc_srvname);
		exit(1);
	}
	switch (level) {
	    case SMBL_VC:
		opt = SMBV_PERMANENT;
		break;
	    case SMBL_SHARE:
		opt = SMBS_PERMANENT;
		break;
	    default:
		smb_error("unknown connection level %d", 0, level);
		exit(1);
	}
	error = smb_ctx_setflags(ctx, level, opt, opt);
	if (error && error != EACCES) {
		smb_error("Can't make connection permanent", error);
		exit(1);
	}
	printf("Connected to %s%s%s\n", ctx->ct_ssn.ioc_user,
	    level == SMBL_SHARE ? "@" : "",
	    level == SMBL_SHARE ? ctx->ct_sh.ioc_share : "");
	return 0;
}

int
cmd_logout(int argc, char *argv[])
{
	struct smb_ctx sctx, *ctx = &sctx;
	int error, opt, level;

	if (argc < 2)
		logout_usage();
	if (smb_ctx_init(ctx, argc, argv, SMBL_VC, SMBL_SHARE, SMB_ST_ANY) != 0)
		exit(1);
	if (smb_ctx_readrc(ctx) != 0)
		exit(1);
	if (smb_rc)
		rc_close(smb_rc);
	while ((opt = getopt(argc, argv, STDPARAM_OPT)) != EOF){
		switch (opt) {
		    case STDPARAM_ARGS:
			error = smb_ctx_opt(ctx, opt, optarg);
			if (error)
				exit(1);
			break;
		    default:
			logout_usage();
			/*NOTREACHED*/
		}
	}
#ifdef APPLE
	error = loadsmbvfs();
	if (error)
		errx(EX_OSERR, "SMB filesystem is not available");
#endif
	ctx->ct_ssn.ioc_opt &= ~SMBVOPT_CREATE;
	ctx->ct_sh.ioc_opt &= ~SMBSOPT_CREATE;
	if (smb_ctx_resolve(ctx) != 0)
		exit(1);
	level = ctx->ct_parsedlevel;
	error = smb_ctx_lookup(ctx, level, 0);
	if (error == ENOENT) {
/*		ctx->ct_ssn.ioc_opt |= SMBCOPT_SINGLE;
		error = smb_ctx_login(ctx);
		if (error == ENOENT) {
			ctx->ct_ssn.ioc_opt |= SMBCOPT_PRIVATE;
			error = smb_ctx_login(ctx);
			if (error == ENOENT) {
				ctx->ct_ssn.ioc_opt &= ~SMBCOPT_SINGLE;
				error = smb_ctx_login(ctx);
			}
		}*/
		if (error) {
			smb_error("There is no connection to %s", error, ctx->ct_ssn.ioc_srvname);
			exit(1);
		}
	}
	if (error)
		exit(1);
	switch (level) {
	    case SMBL_VC:
		opt = SMBV_PERMANENT;
		break;
	    case SMBL_SHARE:
		opt = SMBS_PERMANENT;
		break;
	    default:
		smb_error("unknown connection level %d", 0, level);
		exit(1);
	}
	error = smb_ctx_setflags(ctx, level, opt, 0);
	if (error && error != EACCES) {
		smb_error("Can't release connection", error);
		exit(1);
	}
	printf("Connection unmarked as permanent and will be closed when possible\n");
	exit(0);
}

void
login_usage(void)
{
	printf(
	"usage: smbutil login [-E cs1:cs2] [-I host] [-L locale] [-M crights:srights]\n"
	"               [-N cowner:cgroup/sowner:sgroup] [-P]\n"
	"               [-R retrycount] [-T timeout]\n"
	"               [-W workgroup] //user@server\n");
	exit(1);
}

void
logout_usage(void)
{
	printf("usage: smbutil logout [user@server]\n");
	exit(1);
}