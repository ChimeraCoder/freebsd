
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
/* @(#)rsort.c	2.1 88/08/11 4.0 RPCSRC */
/*
 * rsort.c
 * Client side application which sorts argc, argv.
 */#include <stdio.h>
#include <rpc/rpc.h>
#include "sort.h"

main(argc, argv)
	int argc;
	char **argv;
{
	char *machinename;
	struct sortstrings args, res;
	int i;

	if (argc < 3) {
		fprintf(stderr, "usage: %s machinename [s1 ...]\n", argv[0]);
		exit(1);
	}
	machinename = argv[1];
	args.ss.ss_len = argc - 2;     /* substract off progname, machinename */
	args.ss.ss_val = &argv[2];
	res.ss.ss_val = (char **)NULL;

	if ((i = callrpc(machinename, SORTPROG, SORTVERS, SORT,
	    xdr_sortstrings, &args, xdr_sortstrings, &res)))
	{
	    fprintf(stderr, "%s: call to sort service failed. ", argv[0]);
	    clnt_perrno(i);
	    fprintf(stderr, "\n");
	    exit(1);
	}

	for (i = 0; i < res.ss.ss_len; i++) {
		printf("%s\n", res.ss.ss_val[i]);
	}

	/* should free res here */
	exit(0);
}