
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
#include <stdlib.h>
#include <string.h>

#include "../e_os.h"

#if defined(OPENSSL_NO_DES) && !defined(OPENSSL_NO_MDC2)
#define OPENSSL_NO_MDC2
#endif

#ifdef OPENSSL_NO_MDC2
int main(int argc, char *argv[])
{
    printf("No MDC2 support\n");
    return(0);
}
#else
#include <openssl/evp.h>
#include <openssl/mdc2.h>

#ifdef CHARSET_EBCDIC
#include <openssl/ebcdic.h>
#endif

static unsigned char pad1[16]={
	0x42,0xE5,0x0C,0xD2,0x24,0xBA,0xCE,0xBA,
	0x76,0x0B,0xDD,0x2B,0xD4,0x09,0x28,0x1A
	};

static unsigned char pad2[16]={
	0x2E,0x46,0x79,0xB5,0xAD,0xD9,0xCA,0x75,
	0x35,0xD8,0x7A,0xFE,0xAB,0x33,0xBE,0xE2
	};

int main(int argc, char *argv[])
	{
	int ret=0;
	unsigned char md[MDC2_DIGEST_LENGTH];
	int i;
	EVP_MD_CTX c;
	static char *text="Now is the time for all ";

#ifdef CHARSET_EBCDIC
	ebcdic2ascii(text,text,strlen(text));
#endif

	EVP_MD_CTX_init(&c);
	EVP_DigestInit_ex(&c,EVP_mdc2(), NULL);
	EVP_DigestUpdate(&c,(unsigned char *)text,strlen(text));
	EVP_DigestFinal_ex(&c,&(md[0]),NULL);

	if (memcmp(md,pad1,MDC2_DIGEST_LENGTH) != 0)
		{
		for (i=0; i<MDC2_DIGEST_LENGTH; i++)
			printf("%02X",md[i]);
		printf(" <- generated\n");
		for (i=0; i<MDC2_DIGEST_LENGTH; i++)
			printf("%02X",pad1[i]);
		printf(" <- correct\n");
		ret=1;
		}
	else
		printf("pad1 - ok\n");

	EVP_DigestInit_ex(&c,EVP_mdc2(), NULL);
	/* FIXME: use a ctl function? */
	((MDC2_CTX *)c.md_data)->pad_type=2;
	EVP_DigestUpdate(&c,(unsigned char *)text,strlen(text));
	EVP_DigestFinal_ex(&c,&(md[0]),NULL);

	if (memcmp(md,pad2,MDC2_DIGEST_LENGTH) != 0)
		{
		for (i=0; i<MDC2_DIGEST_LENGTH; i++)
			printf("%02X",md[i]);
		printf(" <- generated\n");
		for (i=0; i<MDC2_DIGEST_LENGTH; i++)
			printf("%02X",pad2[i]);
		printf(" <- correct\n");
		ret=1;
		}
	else
		printf("pad2 - ok\n");

	EVP_MD_CTX_cleanup(&c);
#ifdef OPENSSL_SYS_NETWARE
    if (ret) printf("ERROR: %d\n", ret);
#endif
	EXIT(ret);
	return(ret);
	}
#endif