
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

#include "telnet_locl.h"

RCSID("$Id$");

#if	defined(AUTHENTICATION) || defined(ENCRYPTION)
int
telnet_net_write(unsigned char *str, int len)
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
#if	defined(ENCRYPTION)
	if (encrypt_output)
		ring_encrypt(&netoring, encrypt_output);
	else
		ring_clearto(&netoring);
#endif
}

int
telnet_spin(void)
{
    int ret = 0;

    scheduler_lockout_tty = 1;
    if (Scheduler(0) == -1)
	ret = 1;
    scheduler_lockout_tty = 0;

    return ret;

}

char *
telnet_getenv(const char *val)
{
	return((char *)env_getvalue((unsigned char *)val));
}

char *
telnet_gets(char *prompt, char *result, int length, int echo)
{
	int om = globalmode;
	char *res;

	TerminalNewMode(-1);
	if (echo) {
		printf("%s", prompt);
		res = fgets(result, length, stdin);
	} else if ((res = getpass(prompt))) {
		strlcpy(result, res, length);
		res = result;
	}
	TerminalNewMode(om);
	return(res);
}
#endif