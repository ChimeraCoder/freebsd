
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)publickey.c 1.10 91/03/11 Copyr 1986 Sun Micro";
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * publickey.c
 * Copyright (C) 1986, Sun Microsystems, Inc.
 */

/*
 * Public key lookup routines
 */
#include "namespace.h"
#include <stdio.h>
#include <pwd.h>
#include <rpc/rpc.h>
#include <rpc/key_prot.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>
#include <string.h>
#include <stdlib.h>
#include "un-namespace.h"

#define PKFILE "/etc/publickey"

/*
 * Hack to let ypserv/rpc.nisd use AUTH_DES.
 */
int (*__getpublickey_LOCAL)() = 0;

/*
 * Get somebody's public key
 */
static int
__getpublickey_real(netname, publickey)
	const char *netname;
	char *publickey;
{
	char lookup[3 * HEXKEYBYTES];
	char *p;

	if (publickey == NULL)
		return (0);
	if (!getpublicandprivatekey(netname, lookup))
		return (0);
	p = strchr(lookup, ':');
	if (p == NULL) {
		return (0);
	}
	*p = '\0';
	(void) strncpy(publickey, lookup, HEXKEYBYTES);
	publickey[HEXKEYBYTES] = '\0';
	return (1);
}

/*
 * reads the file /etc/publickey looking for a + to optionally go to the
 * yellow pages
 */

int
getpublicandprivatekey(key, ret)
	const char *key;
	char *ret;
{
	char buf[1024];	/* big enough */
	char *res;
	FILE *fd;
	char *mkey;
	char *mval;

	fd = fopen(PKFILE, "r");
	if (fd == NULL)
		return (0);
	for (;;) {
		res = fgets(buf, sizeof(buf), fd);
		if (res == NULL) {
			fclose(fd);
			return (0);
		}
		if (res[0] == '#')
			continue;
		else if (res[0] == '+') {
#ifdef YP
			char *PKMAP = "publickey.byname";
			char *lookup;
			char *domain;
			int err;
			int len;

			err = yp_get_default_domain(&domain);
			if (err) {
				continue;
			}
			lookup = NULL;
			err = yp_match(domain, PKMAP, key, strlen(key), &lookup, &len);
			if (err) {
#ifdef DEBUG
				fprintf(stderr, "match failed error %d\n", err);
#endif
				continue;
			}
			lookup[len] = 0;
			strcpy(ret, lookup);
			fclose(fd);
			free(lookup);
			return (2);
#else /* YP */
#ifdef DEBUG
			fprintf(stderr,
"Bad record in %s '+' -- NIS not supported in this library copy\n", PKFILE);
#endif /* DEBUG */
			continue;
#endif /* YP */
		} else {
			mkey = strsep(&res, "\t ");
			if (mkey == NULL) {
				fprintf(stderr,
				"Bad record in %s -- %s", PKFILE, buf);
				continue;
			}
			do {
				mval = strsep(&res, " \t#\n");
			} while (mval != NULL && !*mval);
			if (mval == NULL) {
				fprintf(stderr,
			"Bad record in %s val problem - %s", PKFILE, buf);
				continue;
			}
			if (strcmp(mkey, key) == 0) {
				strcpy(ret, mval);
				fclose(fd);
				return (1);
			}
		}
	}
}

int getpublickey(netname, publickey)
	const char *netname;
	char *publickey;
{
	if (__getpublickey_LOCAL != NULL)
		return(__getpublickey_LOCAL(netname, publickey));
	else
		return(__getpublickey_real(netname, publickey));
}