
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
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)keylogin.c 1.4 91/03/11 Copyr 1986 Sun Micro";
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * Copyright (C) 1986, Sun Microsystems, Inc.
 */

/*
 * Set secret key on local machine
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <rpc/rpc.h>
#include <rpc/key_prot.h>

extern int key_setnet(struct key_netstarg *);

int
main(void)
{
	char fullname[MAXNETNAMELEN + 1];
	struct key_netstarg netst;

	if (!getnetname(fullname)) {
		fprintf(stderr, "netname lookup failed -- make sure the ");
		fprintf(stderr, "system domain name is set.\n");
		exit(1);
	}

	if (! getsecretkey(fullname, (char *)&(netst.st_priv_key),
				getpass("Password:"))) {
		fprintf(stderr, "Can't find %s's secret key\n", fullname);
		exit(1);
	}
	if (netst.st_priv_key[0] == 0) {
		fprintf(stderr, "Password incorrect for %s\n", fullname);
		exit(1);
	}

	netst.st_pub_key[0] = 0;
	netst.st_netname = strdup(fullname);

	if (key_setnet(&netst) < 0) {
		fprintf(stderr, "Could not set %s's secret key\n", fullname);
		fprintf(stderr, "Maybe the keyserver is down?\n");
		exit(1);
	}
	exit(0);
	/* NOTREACHED */
}