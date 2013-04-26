
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

#if 0
#ifndef lint
static const char sccsid[] = "@(#)authenc.c	8.1 (Berkeley) 6/6/93";
#endif
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#ifdef	AUTHENTICATION
#ifdef	ENCRYPTION
#include <sys/types.h>
#include <arpa/telnet.h>
#include <pwd.h>
#include <unistd.h>
#include <libtelnet/encrypt.h>
#include <libtelnet/misc.h>

#include "general.h"
#include "ring.h"
#include "externs.h"
#include "defines.h"
#include "types.h"

int
net_write(unsigned char *str, int len)
{
	if (NETROOM() > len) {
		ring_supply_data(&netoring, str, len);
		if (str[0] == IAC && str[1] == SE)
			printsub('>', &str[2], len-2);
		return(len);
	}
	return(0);
}

void
net_encrypt(void)
{
#ifdef	ENCRYPTION
	if (encrypt_output)
		ring_encrypt(&netoring, encrypt_output);
	else
		ring_clearto(&netoring);
#endif	/* ENCRYPTION */
}

int
telnet_spin(void)
{
	return(-1);
}

char *
telnet_getenv(char *val)
{
	return((char *)env_getvalue((unsigned char *)val));
}

char *
telnet_gets(const char *prom, char *result, int length, int echo)
{
	extern int globalmode;
	int om = globalmode;
	char *res;

	TerminalNewMode(-1);
	if (echo) {
		printf("%s", prom);
		res = fgets(result, length, stdin);
	} else if ((res = getpass(prom))) {
		strncpy(result, res, length);
		res = result;
	}
	TerminalNewMode(om);
	return(res);
}
#endif	/* ENCRYPTION */
#endif	/* AUTHENTICATION */