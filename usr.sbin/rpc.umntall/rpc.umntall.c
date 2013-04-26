
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

#ifndef lint
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>

#include <rpc/rpc.h>
#include <rpcsvc/mount.h>

#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mounttab.h"

int verbose;

static int do_umount (char *, char *);
static int do_umntall (char *);
static int is_mounted (char *, char *);
static void usage (void);
int	xdr_dir (XDR *, char *);

int
main(int argc, char **argv) {
	int ch, keep, success, pathlen;
	time_t expire, now;
	char *host, *path;
	struct mtablist *mtab;

	expire = 0;
	host = path = NULL;
	success = keep = verbose = 0;
	while ((ch = getopt(argc, argv, "h:kp:ve:")) != -1)
		switch (ch) {
		case 'h':
			host = optarg;
			break;
		case 'e':
			expire = atoi(optarg);
			break;
		case 'k':
			keep = 1;
			break;
		case 'p':
			path = optarg;
			break;
		case 'v':
			verbose = 1;
			break;
		case '?':
			usage();
		default:
			break;
		}
	argc -= optind;
	argv += optind;

	/* Default expiretime is one day */
	if (expire == 0)
		expire = 86400;
	time(&now);

	/* Read PATH_MOUNTTAB. */
	if (!read_mtab()) {
		if (verbose)
			warnx("no mounttab entries (%s does not exist)",
			    PATH_MOUNTTAB);
		mtabhead = NULL;
	}

	if (host == NULL && path == NULL) {
		/* Check each entry and do any necessary unmount RPCs. */
		for (mtab = mtabhead; mtab != NULL; mtab = mtab->mtab_next) {
			if (*mtab->mtab_host == '\0')
				continue;
			if (mtab->mtab_time + expire < now) {
				/* Clear expired entry. */
				if (verbose)
					warnx("remove expired entry %s:%s",
					    mtab->mtab_host, mtab->mtab_dirp);
				bzero(mtab->mtab_host,
				    sizeof(mtab->mtab_host));
				continue;
			}
			if (keep && is_mounted(mtab->mtab_host,
			    mtab->mtab_dirp)) {
				if (verbose)
					warnx("skip entry %s:%s",
					    mtab->mtab_host, mtab->mtab_dirp);
				continue;
			}
			if (do_umount(mtab->mtab_host, mtab->mtab_dirp)) {
				if (verbose)
					warnx("umount RPC for %s:%s succeeded",
					    mtab->mtab_host, mtab->mtab_dirp);
				/* Remove all entries for this host + path. */
				clean_mtab(mtab->mtab_host, mtab->mtab_dirp,
				    verbose);
			}
		}
		success = 1;
	} else {
		if (host == NULL && path != NULL)
			/* Missing hostname. */
			usage();
		if (path == NULL) {
			/* Do a RPC UMNTALL for this specific host */
			success = do_umntall(host);
			if (verbose && success)
				warnx("umntall RPC for %s succeeded", host);
		} else {
			/* Do a RPC UMNTALL for this specific mount */
			for (pathlen = strlen(path);
			    pathlen > 1 && path[pathlen - 1] == '/'; pathlen--)
				path[pathlen - 1] = '\0';
			success = do_umount(host, path);
			if (verbose && success)
				warnx("umount RPC for %s:%s succeeded", host,
				    path);
		}
		/* If successful, remove any corresponding mounttab entries. */
		if (success)
			clean_mtab(host, path, verbose);
	}
	/* Write and unlink PATH_MOUNTTAB if necessary */
	if (success)
		success = write_mtab(verbose);
	free_mtab();
	exit (success ? 0 : 1);
}

/*
 * Send a RPC_MNT UMNTALL request to hostname.
 * XXX This works for all mountd implementations,
 * but produces a RPC IOERR on non FreeBSD systems.
 */
int
do_umntall(char *hostname) {
	enum clnt_stat clnt_stat;
	struct timeval try;
	CLIENT *clp;

	try.tv_sec = 3;
	try.tv_usec = 0;
	clp = clnt_create_timed(hostname, MOUNTPROG, MOUNTVERS, "udp",
	    &try);
	if (clp == NULL) {
		warnx("%s: %s", hostname, clnt_spcreateerror("MOUNTPROG"));
		return (0);
	}
	clp->cl_auth = authunix_create_default();
	clnt_stat = clnt_call(clp, MOUNTPROC_UMNTALL,
	    (xdrproc_t)xdr_void, (caddr_t)0,
	    (xdrproc_t)xdr_void, (caddr_t)0, try);
	if (clnt_stat != RPC_SUCCESS)
		warnx("%s: %s", hostname, clnt_sperror(clp, "MOUNTPROC_UMNTALL"));
	auth_destroy(clp->cl_auth);
	clnt_destroy(clp);
	return (clnt_stat == RPC_SUCCESS);
}

/*
 * Send a RPC_MNT UMOUNT request for dirp to hostname.
 */
int
do_umount(char *hostname, char *dirp) {
	enum clnt_stat clnt_stat;
	struct timeval try;
	CLIENT *clp;

	try.tv_sec = 3;
	try.tv_usec = 0;
	clp = clnt_create_timed(hostname, MOUNTPROG, MOUNTVERS, "udp",
	    &try);
	if (clp  == NULL) {
		warnx("%s: %s", hostname, clnt_spcreateerror("MOUNTPROG"));
		return (0);
	}
	clp->cl_auth = authsys_create_default();
	clnt_stat = clnt_call(clp, MOUNTPROC_UMNT, (xdrproc_t)xdr_dir, dirp,
	    (xdrproc_t)xdr_void, (caddr_t)0, try);
	if (clnt_stat != RPC_SUCCESS)
		warnx("%s: %s", hostname, clnt_sperror(clp, "MOUNTPROC_UMNT"));
	auth_destroy(clp->cl_auth);
	clnt_destroy(clp);
	return (clnt_stat == RPC_SUCCESS);
}

/*
 * Check if the entry is still/already mounted.
 */
int
is_mounted(char *hostname, char *dirp) {
	struct statfs *mntbuf;
	char name[MNAMELEN + 1];
	size_t bufsize;
	int mntsize, i;

	if (strlen(hostname) + strlen(dirp) >= MNAMELEN)
		return (0);
	snprintf(name, sizeof(name), "%s:%s", hostname, dirp);
	mntsize = getfsstat(NULL, 0, MNT_NOWAIT);
	if (mntsize <= 0)
		return (0);
	bufsize = (mntsize + 1) * sizeof(struct statfs);
	if ((mntbuf = malloc(bufsize)) == NULL)
		err(1, "malloc");
	mntsize = getfsstat(mntbuf, (long)bufsize, MNT_NOWAIT);
	for (i = mntsize - 1; i >= 0; i--) {
		if (strcmp(mntbuf[i].f_mntfromname, name) == 0) {
			free(mntbuf);
			return (1);
		}
	}
	free(mntbuf);
	return (0);
}

/*
 * xdr routines for mount rpc's
 */
int
xdr_dir(XDR *xdrsp, char *dirp) {
	return (xdr_string(xdrsp, &dirp, MNTPATHLEN));
}

static void
usage() {
	(void)fprintf(stderr, "%s\n",
	    "usage: rpc.umntall [-kv] [-e expire] [-h host] [-p path]");
	exit(1);
}