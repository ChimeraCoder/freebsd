
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpc/des_crypt.h>

static char hex[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

static char hexval( char );
static void bin2hex( int, unsigned char *, char * );
static void hex2bin( int, char *, char * );
void passwd2des( char *, char * );

/*
 * Encrypt a secret key given passwd
 * The secret key is passed and returned in hex notation.
 * Its length must be a multiple of 16 hex digits (64 bits).
 */
int
xencrypt(char *secret, char *passwd)
{
	char key[8];
	char ivec[8];
	char *buf;
	int err;
	int len;

	len = strlen(secret) / 2;
	if ((buf = malloc((unsigned)len)) == NULL) {
		return(0);
	}

	hex2bin(len, secret, buf);
	passwd2des(passwd, key);
	bzero(ivec, 8);

	err = cbc_crypt(key, buf, len, DES_ENCRYPT | DES_HW, ivec);
	if (DES_FAILED(err)) {	
		free(buf);
		return (0);
	}
	bin2hex(len, (unsigned char *) buf, secret);
	free(buf);
	return (1);
}

/*
 * Decrypt secret key using passwd
 * The secret key is passed and returned in hex notation.
 * Once again, the length is a multiple of 16 hex digits
 */
int
xdecrypt(char *secret, char *passwd)
{
	char key[8];
	char ivec[8];
	char *buf;
	int err;
	int len;

	len = strlen(secret) / 2;
	if ((buf = malloc((unsigned)len)) == NULL) {
		return(0);
	}

	hex2bin(len, secret, buf);
	passwd2des(passwd, key);	
	bzero(ivec, 8);

	err = cbc_crypt(key, buf, len, DES_DECRYPT | DES_HW, ivec);
	if (DES_FAILED(err)) {
		free(buf);
		return (0);
	}
	bin2hex(len, (unsigned char *) buf, secret);
	free(buf);
	return (1);
}


/*
 * Turn password into DES key
 */
void
passwd2des(char *pw, char *key)
{
	int i;

	bzero(key, 8);
	for (i = 0; *pw; i = (i+1)%8) {
		key[i] ^= *pw++ << 1;
	}
	des_setparity(key);
}



/*
 * Hex to binary conversion
 */
static void
hex2bin(int len, char *hexnum, char *binnum)
{
	int i;

	for (i = 0; i < len; i++) {
		*binnum++ = 16 * hexval(hexnum[2*i]) + hexval(hexnum[2*i+1]);
	}
}

/*
 * Binary to hex conversion
 */
static void
bin2hex(int len, unsigned char *binnum, char *hexnum)
{
	int i;
	unsigned val;

	for (i = 0; i < len; i++) {
		val = binnum[i];
		hexnum[i*2] = hex[val >> 4];
		hexnum[i*2+1] = hex[val & 0xf];
	}
	hexnum[len*2] = 0;
}

static char
hexval(char c)
{
	if (c >= '0' && c <= '9') {
		return (c - '0');
	} else if (c >= 'a' && c <= 'z') {
		return (c - 'a' + 10);
	} else if (c >= 'A' && c <= 'Z') {
		return (c - 'A' + 10);
	} else {
		return (-1);
	}
}