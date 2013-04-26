
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

#include <sys/param.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#include <sys/linker.h>

#include <net/route.h>

#include <netgraph.h>
#include <netgraph/ng_message.h>
#include <netgraph/ng_socket.h>
#include <netgraph/ng_socketvar.h>

#include <nlist.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include "netstat.h"

static	int first = 1;
static	int csock = -1;

void
netgraphprotopr(u_long off, const char *name, int af1 __unused,
    int proto __unused)
{
	struct ngpcb *this, *next;
	struct ngpcb ngpcb;
	struct socket sockb;
	int debug = 1;

	/* If symbol not found, try looking in the KLD module */
	if (off == 0) {
		const char *const modname = "ng_socket.ko";
/* XXX We should get "mpath" from "sysctl kern.module_path" */
		const char *mpath[] = { "/", "/boot/", "/modules/", NULL };
		struct nlist sym[] = { { .n_name = "_ngsocklist" },
				       { .n_name = NULL } };
		const char **pre;
		struct kld_file_stat ks;
		int fileid;

		/* Can't do this for core dumps. */
		if (!live)
			return;

		/* See if module is loaded */
		if ((fileid = kldfind(modname)) < 0) {
			if (debug)
				warn("kldfind(%s)", modname);
			return;
		}

		/* Get module info */
		memset(&ks, 0, sizeof(ks));
		ks.version = sizeof(struct kld_file_stat);
		if (kldstat(fileid, &ks) < 0) {
			if (debug)
				warn("kldstat(%d)", fileid);
			return;
		}

		/* Get symbol table from module file */
		for (pre = mpath; *pre; pre++) {
			char path[MAXPATHLEN];

			snprintf(path, sizeof(path), "%s%s", *pre, modname);
			if (nlist(path, sym) == 0)
				break;
		}

		/* Did we find it? */
		if (sym[0].n_value == 0) {
			if (debug)
				warnx("%s not found", modname);
			return;
		}

		/* Symbol found at load address plus symbol offset */
		off = (u_long) ks.address + sym[0].n_value;
	}

	/* Get pointer to first socket */
	kread(off, (char *)&this, sizeof(this));

	/* Get my own socket node */
	if (csock == -1)
		NgMkSockNode(NULL, &csock, NULL);

	for (; this != NULL; this = next) {
		u_char rbuf[sizeof(struct ng_mesg) + sizeof(struct nodeinfo)];
		struct ng_mesg *resp = (struct ng_mesg *) rbuf;
		struct nodeinfo *ni = (struct nodeinfo *) resp->data;
		char path[64];

		/* Read in ngpcb structure */
		kread((u_long)this, (char *)&ngpcb, sizeof(ngpcb));
		next = LIST_NEXT(&ngpcb, socks);

		/* Read in socket structure */
		kread((u_long)ngpcb.ng_socket, (char *)&sockb, sizeof(sockb));

		/* Check type of socket */
		if (strcmp(name, "ctrl") == 0 && ngpcb.type != NG_CONTROL)
			continue;
		if (strcmp(name, "data") == 0 && ngpcb.type != NG_DATA)
			continue;

		/* Do headline */
		if (first) {
			printf("Netgraph sockets\n");
			if (Aflag)
				printf("%-8.8s ", "PCB");
			printf("%-5.5s %-6.6s %-6.6s %-14.14s %s\n",
			    "Type", "Recv-Q", "Send-Q",
			    "Node Address", "#Hooks");
			first = 0;
		}

		/* Show socket */
		if (Aflag)
			printf("%8lx ", (u_long) this);
		printf("%-5.5s %6u %6u ",
		    name, sockb.so_rcv.sb_cc, sockb.so_snd.sb_cc);

		/* Get info on associated node */
		if (ngpcb.node_id == 0 || csock == -1)
			goto finish;
		snprintf(path, sizeof(path), "[%x]:", ngpcb.node_id);
		if (NgSendMsg(csock, path,
		    NGM_GENERIC_COOKIE, NGM_NODEINFO, NULL, 0) < 0)
			goto finish;
		if (NgRecvMsg(csock, resp, sizeof(rbuf), NULL) < 0)
			goto finish;

		/* Display associated node info */
		if (*ni->name != '\0')
			snprintf(path, sizeof(path), "%s:", ni->name);
		printf("%-14.14s %4d", path, ni->hooks);
finish:
		putchar('\n');
	}
}