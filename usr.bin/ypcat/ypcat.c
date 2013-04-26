
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
#include <sys/types.h>
#include <sys/socket.h>

#include <rpc/rpc.h>
#include <rpc/xdr.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>

#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct ypalias {
	char *alias, *name;
} ypaliases[] = {
	{ "passwd", "passwd.byname" },
	{ "master.passwd", "master.passwd.byname" },
	{ "shadow", "shadow.byname" },
	{ "group", "group.byname" },
	{ "networks", "networks.byaddr" },
	{ "hosts", "hosts.byaddr" },
	{ "protocols", "protocols.bynumber" },
	{ "services", "services.byname" },
	{ "aliases", "mail.aliases" },
	{ "ethers", "ethers.byname" },
};

int key;

static void
usage(void)
{
	fprintf(stderr, "%s\n%s\n",
		"usage: ypcat [-kt] [-d domainname] mapname",
		"       ypcat -x");
	exit(1);
}

static int
printit(unsigned long instatus, char *inkey, int inkeylen, char *inval, int invallen, void *dummy __unused)
{
	if (instatus != YP_TRUE)
		return (instatus);
	if (key)
		printf("%*.*s ", inkeylen, inkeylen, inkey);
	printf("%*.*s\n", invallen, invallen, inval);
	return (0);
}

int
main(int argc, char *argv[])
{
	char *domainname = NULL;
	struct ypall_callback ypcb;
	char *inmap;
	int notrans;
	int c, r;
	u_int i;

	notrans = key = 0;

	while ((c = getopt(argc, argv, "xd:kt")) != -1)
		switch (c) {
		case 'x':
			for (i = 0; i<sizeof ypaliases/sizeof ypaliases[0]; i++)
				printf("Use \"%s\" for \"%s\"\n",
					ypaliases[i].alias,
					ypaliases[i].name);
			exit(0);
		case 'd':
			domainname = optarg;
			break;
		case 't':
			notrans++;
			break;
		case 'k':
			key++;
			break;
		default:
			usage();
		}

	if (optind + 1 != argc)
		usage();

	if (!domainname)
		yp_get_default_domain(&domainname);

	inmap = argv[optind];
	for (i = 0; (!notrans) && i<sizeof ypaliases/sizeof ypaliases[0]; i++)
		if (strcmp(inmap, ypaliases[i].alias) == 0)
			inmap = ypaliases[i].name;
	ypcb.foreach = printit;
	ypcb.data = NULL;

	r = yp_all(domainname, inmap, &ypcb);
	switch (r) {
	case 0:
		break;
	case YPERR_YPBIND:
		errx(1, "not running ypbind");
	default:
		errx(1, "no such map %s. reason: %s", inmap, yperr_string(r));
	}
	exit(0);
}