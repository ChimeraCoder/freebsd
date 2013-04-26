
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
#include <sys/socket.h>
#include <err.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <sysexits.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <cflib.h>

#include <netsmb/netbios.h>
#include <netsmb/smb_lib.h>
#include <netsmb/nb_lib.h>
#include <netsmb/smb_conn.h>

#include "common.h"


int
cmd_lookup(int argc, char *argv[])
{
	struct nb_ctx *ctx;
	struct sockaddr *sap;
	char *hostname;
	int error, opt;

	if (argc < 2)
		lookup_usage();
	error = nb_ctx_create(&ctx);
	if (error) {
		smb_error("unable to create nbcontext", error);
		exit(1);
	}
	if (smb_open_rcfile() == 0) {
		if (nb_ctx_readrcsection(smb_rc, ctx, "default", 0) != 0)
			exit(1);
		rc_close(smb_rc);
	}
	while ((opt = getopt(argc, argv, "w:")) != EOF) {
		switch(opt){
		    case 'w':
			nb_ctx_setns(ctx, optarg);
			break;
		    default:
			lookup_usage();
			/*NOTREACHED*/
		}
	}
	if (optind >= argc)
		lookup_usage();
	if (nb_ctx_resolve(ctx) != 0)
		exit(1);
	hostname = argv[argc - 1];
/*	printf("Looking for %s...\n", hostname);*/
	error = nbns_resolvename(hostname, ctx, &sap);
	if (error) {
		smb_error("unable to resolve %s", error, hostname);
		exit(1);
	}
	printf("Got response from %s\n", inet_ntoa(ctx->nb_lastns.sin_addr));
	printf("IP address of %s: %s\n", hostname, inet_ntoa(((struct sockaddr_in*)sap)->sin_addr));
	return 0;
}


void
lookup_usage(void)
{
	printf("usage: smbutil lookup [-w host] name\n");
	exit(1);
}