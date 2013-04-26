
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

/*
 * unset the secret key on local machine
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <rpc/key_prot.h>

int
main(int argc, char **argv)
{
	static char secret[HEXKEYBYTES + 1];

	if (geteuid() == 0) {
		if ((argc != 2 ) || (strcmp(argv[1], "-f") != 0)) {
			fprintf(stderr,
"keylogout by root would break all servers that use secure rpc!\n");
			fprintf(stderr,
"root may use keylogout -f to do this (at your own risk)!\n");
			exit(1);
		}
	}

	if (key_setsecret(secret) < 0) {
		fprintf(stderr, "Could not unset your secret key.\n");
		fprintf(stderr, "Maybe the keyserver is down?\n");
		exit(1);
	}
	exit(0);
	/* NOTREACHED */
}