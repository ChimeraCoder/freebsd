
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

#ifndef lint
#if 0
static const char sccsid[] = "@(#)misc.c	8.1 (Berkeley) 6/4/93";
#endif
#endif /* not lint */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "misc.h"
#ifdef	AUTHENTICATION
#include "auth.h"
#endif
#ifdef	ENCRYPTION
#include "encrypt.h"
#endif	/* ENCRYPTION */

char *RemoteHostName;
char *LocalHostName;
char *UserNameRequested = 0;
int ConnectedCount = 0;

#ifndef AUTHENTICATION
#define undef1 __unused
#else
#define undef1
#endif

void
auth_encrypt_init(char *local, char *remote, const char *name undef1, int server undef1)
{
	RemoteHostName = remote;
	LocalHostName = local;
#ifdef	AUTHENTICATION
	auth_init(name, server);
#endif
#ifdef	ENCRYPTION
	encrypt_init(name, server);
#endif	/* ENCRYPTION */
	if (UserNameRequested) {
		free(UserNameRequested);
		UserNameRequested = 0;
	}
}

#ifdef	ENCRYPTION
void
auth_encrypt_user(char *name)
{
	if (UserNameRequested)
		free(UserNameRequested);
	UserNameRequested = name ? strdup(name) : 0;
}

/* ARGSUSED */
void
auth_encrypt_connect(int cnt __unused)
{
}
#endif	/* ENCRYPTION */

void
printd(const unsigned char *data, int cnt)
{
	if (cnt > 16)
		cnt = 16;
	while (cnt-- > 0) {
		printf(" %02x", *data);
		++data;
	}
}