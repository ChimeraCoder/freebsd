
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/endian.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/iconv.h>
#include <err.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <sysexits.h>

#include <cflib.h>

#include <netsmb/smb_lib.h>
#include <netsmb/smb_conn.h>
#include <netsmb/smb_rap.h>

#include "common.h"

static char *shtype[] = {
	"disk",
	"printer",
	"comm",		/* Communications device */
	"pipe",		/* IPC Inter process communication */
	"unknown"
};

int
cmd_view(int argc, char *argv[])
{
	struct smb_ctx sctx, *ctx = &sctx;
	struct smb_share_info_1 *rpbuf, *ep;
	char *cp;
	u_int32_t remark;
	u_int16_t type;
	int error, opt, bufsize, i, entries, total;
	

	if (argc < 2)
		view_usage();
	if (smb_ctx_init(ctx, argc, argv, SMBL_VC, SMBL_VC, SMB_ST_ANY) != 0)
		exit(1);
	if (smb_ctx_readrc(ctx) != 0)
		exit(1);
	if (smb_rc)
		rc_close(smb_rc);
	while ((opt = getopt(argc, argv, STDPARAM_OPT)) != EOF) {
		switch(opt){
		    case STDPARAM_ARGS:
			error = smb_ctx_opt(ctx, opt, optarg);
			if (error)
				exit(1);
			break;
		    default:
			view_usage();
			/*NOTREACHED*/
		}
	}
#ifdef APPLE
	if (loadsmbvfs())
		errx(EX_OSERR, "SMB filesystem is not available");
#endif
	smb_ctx_setshare(ctx, "IPC$", SMB_ST_ANY);
	if (smb_ctx_resolve(ctx) != 0)
		exit(1);
	error = smb_ctx_lookup(ctx, SMBL_SHARE, SMBLK_CREATE);
	if (error) {
		smb_error("could not login to server %s", error, ctx->ct_ssn.ioc_srvname);
		exit(1);
	}
	printf("Share        Type       Comment\n");
	printf("-------------------------------\n");
	bufsize = 0xffe0; /* samba notes win2k bug with 65535 */
	rpbuf = malloc(bufsize);
	error = smb_rap_NetShareEnum(ctx, 1, rpbuf, bufsize, &entries, &total);
	if (error &&
	    error != (SMB_ERROR_MORE_DATA | SMB_RAP_ERROR)) {
		smb_error("unable to list resources", error);
		exit(1);
	}
	for (ep = rpbuf, i = 0; i < entries; i++, ep++) {
		type = le16toh(ep->shi1_type);
		remark = le32toh(ep->shi1_remark);
		remark &= 0xFFFF;

		cp = (char*)rpbuf + remark;
		printf("%-12s %-10s %s\n", ep->shi1_netname,
		    shtype[min(type, sizeof shtype / sizeof(char *) - 1)],
		    remark ? nls_str_toloc(cp, cp) : "");
	}
	printf("\n%d shares listed from %d available\n", entries, total);
	free(rpbuf);
	return 0;
}


void
view_usage(void)
{
	printf(
	"usage: smbutil view [connection options] //[user@]server\n"
	);
	exit(1);
}