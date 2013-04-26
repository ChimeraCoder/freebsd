
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

#include <config.h>

RCSID("$Id$");

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <roken.h>
#ifdef SOCKS
#include <socks.h>
#endif
#include "misc.h"
#include "auth.h"
#include "encrypt.h"


const char *RemoteHostName;
const char *LocalHostName;
char *UserNameRequested = 0;
int ConnectedCount = 0;

void
auth_encrypt_init(const char *local, const char *remote, const char *name,
		  int server)
{
    RemoteHostName = remote;
    LocalHostName = local;
#ifdef AUTHENTICATION
    auth_init(name, server);
#endif
#ifdef ENCRYPTION
    encrypt_init(name, server);
#endif
    if (UserNameRequested) {
	free(UserNameRequested);
	UserNameRequested = 0;
    }
}

void
auth_encrypt_user(const char *name)
{
    if (UserNameRequested)
	free(UserNameRequested);
    UserNameRequested = name ? strdup(name) : 0;
}

void
auth_encrypt_connect(int cnt)
{
}

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