
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
static const char sccsid[] = "@(#)authenc.c	8.2 (Berkeley) 5/30/95";
#endif
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#ifdef	AUTHENTICATION
#ifdef	ENCRYPTION
/* Above "#ifdef"s actually "or"'ed together. XXX MarkM
 */
#include "telnetd.h"
#include <libtelnet/misc.h>

int
net_write(unsigned char *str, int len)
{
	if (nfrontp + len < netobuf + BUFSIZ) {
		output_datalen(str, len);
		return(len);
	}
	return(0);
}

void
net_encrypt(void)
{
#ifdef	ENCRYPTION
	char *s = (nclearto > nbackp) ? nclearto : nbackp;
	if (s < nfrontp && encrypt_output) {
		(*encrypt_output)((unsigned char *)s, nfrontp - s);
	}
	nclearto = nfrontp;
#endif /* ENCRYPTION */
}

int
telnet_spin(void)
{
	ttloop();
	return(0);
}

char *
telnet_getenv(char *val)
{
	return(getenv(val));
}

char *
telnet_gets(const char *prompt __unused, char *result __unused, int length __unused, int echo __unused)
{
	return(NULL);
}
#endif	/* ENCRYPTION */
#endif	/* AUTHENTICATION */