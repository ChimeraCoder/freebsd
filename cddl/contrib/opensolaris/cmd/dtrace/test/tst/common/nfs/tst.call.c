
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

/*
 * Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <strings.h>
#include <rpc/rpc.h>

#include "rpcsvc/nfs4_prot.h"

int nfs4_skip_bytes;

/*
 * The waiting() function returns the value passed in, until something
 * external modifies it.  In this case, the D script tst.call.d will
 * modify the value of *a, and thus break the while loop in dotest().
 *
 * This serves the purpose of not making the RPC calls until tst.call.d
 * is active.  Thus, the probes in tst.call.d can fire as a result of
 * the RPC call in dotest().
 */

int
waiting(volatile int *a)
{
	return (*a);
}

int
dotest(void)
{
	CLIENT *client;
	AUTH *auth;
	COMPOUND4args args;
	COMPOUND4res res;
	enum clnt_stat status;
	struct timeval timeout;
	nfs_argop4 arg[1];
	char *tag = "dtrace test";
	volatile int a = 0;

	while (waiting(&a) == 0)
		continue;

	timeout.tv_sec = 30;
	timeout.tv_usec = 0;

	client = clnt_create("localhost", NFS4_PROGRAM, NFS_V4, "tcp");
	if (client == NULL) {
		clnt_pcreateerror("test");
		return (1);
	}
	auth = authsys_create_default();
	client->cl_auth = auth;
	args.minorversion = 0;
	args.tag.utf8string_len = strlen(tag);
	args.tag.utf8string_val = tag;
	args.argarray.argarray_len = sizeof (arg) / sizeof (nfs_argop4);
	args.argarray.argarray_val = arg;

	arg[0].argop = OP_PUTROOTFH;
	/* no need to manipulate nfs_argop4_u */

	bzero(&res, sizeof (res));

	status = clnt_call(client, NFSPROC4_COMPOUND,
	    xdr_COMPOUND4args, (caddr_t)&args,
	    xdr_COMPOUND4res, (caddr_t)&res,
	    timeout);
	if (status != RPC_SUCCESS) {
		clnt_perror(client, "test");
		return (2);
	}

	return (0);
}

/*ARGSUSED*/
int
main(int argc, char **argv)
{
	char shareline[BUFSIZ], unshareline[BUFSIZ];
	int rc;

	(void) snprintf(shareline, sizeof (shareline),
	    "mkdir /tmp/nfsv4test.%d ; share /tmp/nfsv4test.%d", getpid(),
	    getpid());
	(void) snprintf(unshareline, sizeof (unshareline),
	    "unshare /tmp/nfsv4test.%d ; rmdir /tmp/nfsv4test.%d", getpid(),
	    getpid());

	(void) system(shareline);
	rc = dotest();
	(void) system(unshareline);

	return (rc);
}