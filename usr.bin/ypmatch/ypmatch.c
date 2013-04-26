
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
	{ "hosts", "hosts.byname" },
	{ "protocols", "protocols.bynumber" },
	{ "services", "services.byname" },
	{ "aliases", "mail.aliases" },
	{ "ethers", "ethers.byname" },
};

static void
usage(void)
{
	fprintf(stderr, "%s\n%s\n",
		"usage: ypmatch [-kt] [-d domainname] key ... mapname",
		"       ypmatch -x");
	exit(1);
}

int
main(int argc, char *argv[])
{
	char *domainname = NULL;
	char *inkey, *inmap, *outbuf;
	int outbuflen, key, notrans;
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

	if ((argc-optind) < 2)
		usage();

	if (!domainname)
		yp_get_default_domain(&domainname);

	inmap = argv[argc-1];
	for (i = 0; (!notrans) && i<sizeof ypaliases/sizeof ypaliases[0]; i++)
		if (strcmp(inmap, ypaliases[i].alias) == 0)
			inmap = ypaliases[i].name;
	for (; optind < argc-1; optind++) {
		inkey = argv[optind];

		r = yp_match(domainname, inmap, inkey,
			strlen(inkey), &outbuf, &outbuflen);
		switch (r) {
		case 0:
			if (key)
				printf("%s ", inkey);
			printf("%*.*s\n", outbuflen, outbuflen, outbuf);
			break;
		case YPERR_YPBIND:
			errx(1, "not running ypbind");
		default:
			errx(1, "can't match key %s in map %s. reason: %s",
				inkey, inmap, yperr_string(r));
		}
	}
	exit(0);
}