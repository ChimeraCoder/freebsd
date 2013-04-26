
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

#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)secretkey.c 1.8 91/03/11 Copyr 1986 Sun Micro";
#endif

/*
 * secretkey.c
 * Copyright (C) 1986, Sun Microsystems, Inc.
 */

/*
 * Secret key lookup routines
 */
#include <stdio.h>
#include <pwd.h>
#include <rpc/rpc.h>
#include <rpc/key_prot.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>
#include <string.h>

extern int xdecrypt( char *, char * );

/*
 * Get somebody's encrypted secret key from the database, using the given
 * passwd to decrypt it.
 */
int
getsecretkey(char *netname, char *secretkey, char *passwd)
{
	char lookup[3 * HEXKEYBYTES];
	char *p;

	if (secretkey == NULL)
		return (0);
	if (!getpublicandprivatekey(netname, lookup))
		return (0);
	p = strchr(lookup, ':');
	if (p == NULL) {
		return (0);
	}
	p++;
	if (!xdecrypt(p, passwd)) {
		return (0);
	}
	if (memcmp(p, p + HEXKEYBYTES, KEYCHECKSUMSIZE) != 0) {
		secretkey[0] = '\0';
		return (1);
	}
	p[HEXKEYBYTES] = '\0';
	(void) strncpy(secretkey, p, HEXKEYBYTES);
	secretkey[HEXKEYBYTES] = '\0';
	return (1);
}