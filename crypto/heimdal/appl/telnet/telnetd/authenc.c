
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

#include "telnetd.h"

RCSID("$Id$");

#ifdef AUTHENTICATION

int
telnet_net_write(unsigned char *str, int len)
{
    if (nfrontp + len < netobuf + BUFSIZ) {
	memmove(nfrontp, str, len);
	nfrontp += len;
	return(len);
    }
    return(0);
}

void
net_encrypt(void)
{
#ifdef ENCRYPTION
    char *s = (nclearto > nbackp) ? nclearto : nbackp;
    if (s < nfrontp && encrypt_output) {
	(*encrypt_output)((unsigned char *)s, nfrontp - s);
    }
    nclearto = nfrontp;
#endif
}

int
telnet_spin(void)
{
    return ttloop();
}

char *
telnet_getenv(const char *val)
{
    return(getenv(val));
}

char *
telnet_gets(char *prompt, char *result, int length, int echo)
{
    return NULL;
}
#endif