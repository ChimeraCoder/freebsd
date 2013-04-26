
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
/* @(#)rls.c	2.2 88/08/12 4.0 RPCSRC */
/*
 * rls.c: Remote directory listing client
 */#include <stdio.h>
#include <rpc/rpc.h>		/* always need this */
#include "dir.h"		/* need this too: will be generated by rpcgen*/

extern int errno;

main(argc, argv)
	int argc;
	char *argv[];
{
	CLIENT *cl;
	char *server;
	char *dir;
	readdir_res *result;
	namelist nl;


	if (argc != 3) {
		fprintf(stderr, "usage: %s host directory\n", argv[0]);
		exit(1);
	}

	/*
	 * Remember what our command line arguments refer to
	 */
	server = argv[1];
	dir = argv[2];

	/*
	 * Create client "handle" used for calling DIRPROG on the
	 * server designated on the command line. We tell the rpc package
	 * to use the "tcp" protocol when contacting the server.
	 */
	cl = clnt_create(server, DIRPROG, DIRVERS, "tcp");
	if (cl == NULL) {
		/*
		 * Couldn't establish connection with server.
		 * Print error message and die.
		 */
		clnt_pcreateerror(server);
		exit(1);
	}

	/*
	 * Call the remote procedure "readdir" on the server
	 */
	result = readdir_1(&dir, cl);
	if (result == NULL) {
		/*
		 * An error occurred while calling the server.
	 	 * Print error message and die.
		 */
		clnt_perror(cl, server);
		exit(1);
	}

	/*
	 * Okay, we successfully called the remote procedure.
	 */
	if (result->errno != 0) {
		/*
		 * A remote system error occurred.
		 * Print error message and die.
		 */
		errno = result->errno;
		perror(dir);
		exit(1);
	}

	/*
	 * Successfuly got a directory listing.
	 * Print it out.
	 */
	for (nl = result->readdir_res_u.list; nl != NULL; nl = nl->next) {
		printf("%s\n", nl->name);
	}
}