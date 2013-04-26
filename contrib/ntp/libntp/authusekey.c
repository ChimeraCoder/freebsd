
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
 * authusekey - decode a key from ascii and use it
 */#include <stdio.h>
#include <ctype.h>

#include "ntp_types.h"
#include "ntp_string.h"
#include "ntp_stdlib.h"

/*
 * Types of ascii representations for keys.  "Standard" means a 64 bit
 * hex number in NBS format, i.e. with the low order bit of each byte
 * a parity bit.  "NTP" means a 64 bit key in NTP format, with the
 * high order bit of each byte a parity bit.  "Ascii" means a 1-to-8
 * character string whose ascii representation is used as the key.
 */

#define	KEY_TYPE_MD5	4

int
authusekey(
	keyid_t keyno,
	int keytype,
	const u_char *str
	)
{
	const u_char *cp;
	int len;

	cp = str;
	len = strlen((const char *)cp);
	if (len == 0)
	    return 0;

	switch(keytype) {
	    case KEY_TYPE_MD5:
		MD5auth_setkey(keyno, str, (int)strlen((const char *)str));
		break;

	    default:
		/* Oh, well */
		return 0;
	}

	return 1;
}