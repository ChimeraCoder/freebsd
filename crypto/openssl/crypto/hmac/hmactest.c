
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../e_os.h"

#ifdef OPENSSL_NO_HMAC
int main(int argc, char *argv[])
{
    printf("No HMAC support\n");
    return(0);
}
#else
#include <openssl/hmac.h>
#ifndef OPENSSL_NO_MD5
#include <openssl/md5.h>
#endif

#ifdef CHARSET_EBCDIC
#include <openssl/ebcdic.h>
#endif

#ifndef OPENSSL_NO_MD5
static struct test_st
	{
	unsigned char key[16];
	int key_len;
	unsigned char data[64];
	int data_len;
	unsigned char *digest;
	} test[4]={
	{	"",
		0,
		"More text test vectors to stuff up EBCDIC machines :-)",
		54,
		(unsigned char *)"e9139d1e6ee064ef8cf514fc7dc83e86",
	},{	{0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,
		 0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,},
		16,
		"Hi There",
		8,
		(unsigned char *)"9294727a3638bb1c13f48ef8158bfc9d",
	},{	"Jefe",
		4,
		"what do ya want for nothing?",
		28,
		(unsigned char *)"750c783e6ab0b503eaa86e310a5db738",
	},{
		{0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,
		 0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,},
		16,
		{0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,
		 0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,
		 0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,
		 0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,
		 0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,
		 0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,
		 0xdd,0xdd},
		50,
		(unsigned char *)"56be34521d144c88dbb8c733f0e8b3f6",
	},
	};
#endif

static char *pt(unsigned char *md);
int main(int argc, char *argv[])
	{
#ifndef OPENSSL_NO_MD5
	int i;
	char *p;
#endif
	int err=0;

#ifdef OPENSSL_NO_MD5
	printf("test skipped: MD5 disabled\n");
#else

#ifdef CHARSET_EBCDIC
	ebcdic2ascii(test[0].data, test[0].data, test[0].data_len);
	ebcdic2ascii(test[1].data, test[1].data, test[1].data_len);
	ebcdic2ascii(test[2].key,  test[2].key,  test[2].key_len);
	ebcdic2ascii(test[2].data, test[2].data, test[2].data_len);
#endif

	for (i=0; i<4; i++)
		{
		p=pt(HMAC(EVP_md5(),
			test[i].key, test[i].key_len,
			test[i].data, test[i].data_len,
			NULL,NULL));

		if (strcmp(p,(char *)test[i].digest) != 0)
			{
			printf("error calculating HMAC on %d entry'\n",i);
			printf("got %s instead of %s\n",p,test[i].digest);
			err++;
			}
		else
			printf("test %d ok\n",i);
		}
#endif /* OPENSSL_NO_MD5 */
	EXIT(err);
	return(0);
	}

#ifndef OPENSSL_NO_MD5
static char *pt(unsigned char *md)
	{
	int i;
	static char buf[80];

	for (i=0; i<MD5_DIGEST_LENGTH; i++)
		sprintf(&(buf[i*2]),"%02x",md[i]);
	return(buf);
	}
#endif
#endif