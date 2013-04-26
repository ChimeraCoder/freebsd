
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
#include <sys/linker.h>
#include <sys/module.h>
#include <sys/mount.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/vnode.h>

#include <netinet/in.h>

#include <nfs/nfssvc.h>

#include <fs/nfs/rpcv2.h>
#include <fs/nfs/nfsproto.h>
#include <fs/nfs/nfskpiport.h>
#include <fs/nfs/nfs.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <paths.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void usage(void);

int
main(int argc, char **argv)
{
	char *cp;
	u_char val;
	int cnt, even;
	struct nfsd_clid revoke_handle;

	if (modfind("nfsd") < 0)
		errx(1, "nfsd not loaded - self terminating");
	if (argc != 2)
		usage();
	cnt = 0;
	cp = argv[1];
	if (strlen(cp) % 2)
		even = 0;
	else
		even = 1;
	val = 0;
	while (*cp) {
		if (*cp >= '0' && *cp <= '9')
			val += (u_char)(*cp - '0');
		else if (*cp >= 'A' && *cp <= 'F')
			val += ((u_char)(*cp - 'A')) + 0xa;
		else if (*cp >= 'a' && *cp <= 'f')
			val += ((u_char)(*cp - 'a')) + 0xa;
		else
			errx(1, "Non hexadecimal digit in %s", argv[1]);
		if (even) {
			val <<= 4;
			even = 0;
		} else {
			revoke_handle.nclid_id[cnt++] = val;
			if (cnt > NFSV4_OPAQUELIMIT)
				errx(1, "Clientid %s, loo long", argv[1]);
			val = 0;
			even = 1;
		}
		cp++;
	}

	/*
	 * Do the revocation system call.
	 */
	revoke_handle.nclid_idlen = cnt;
#ifdef DEBUG
	printf("Idlen=%d\n", revoke_handle.nclid_idlen);
	for (cnt = 0; cnt < revoke_handle.nclid_idlen; cnt++)
		printf("%02x", revoke_handle.nclid_id[cnt]);
	printf("\n");
#else
	if (nfssvc(NFSSVC_ADMINREVOKE, &revoke_handle) < 0)
		err(1, "Admin revoke failed");
#endif
}

static void
usage(void)
{

	errx(1, "Usage: nfsrevoke <ClientID>");
}