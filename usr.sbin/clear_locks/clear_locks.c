
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <rpc/rpc.h>
#include <rpcsvc/nlm_prot.h>

int
main(int argc, char **argv)
{
	enum clnt_stat stat;
	char *hostname;
	nlm4_notify notify;

	if (argc != 2) {
		fprintf(stderr, "Usage: clear_locks <hostname>\n");
		exit(1);
	}
	hostname = argv[1];

	if (geteuid() != 0) {
		fprintf(stderr, "clear_locks: must be root\n");
		exit(1);
	}

	notify.name = hostname;
	notify.state = 0;
	stat = rpc_call("localhost", NLM_PROG, NLM_VERS4, NLM4_FREE_ALL,
	    (xdrproc_t) xdr_nlm4_notify, (void *) &notify,
	    (xdrproc_t) xdr_void, NULL, NULL);

	if (stat != RPC_SUCCESS) {
		clnt_perrno(stat);
		exit(1);
	}
	fprintf(stderr, "clear_locks: cleared locks for hostname %s\n",
	    hostname);

	return (0);
}