
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

#if defined(OPENSSL_NO_SHA) || defined(OPENSSL_NO_SHA0)
int main(int argc, char *argv[])
{
    printf("No SHA0 support\n");
    return(0);
}
#else
#include <openssl/evp.h>
#include <openssl/sha.h>

#ifdef CHARSET_EBCDIC
#include <openssl/ebcdic.h>
#endif

#define SHA_0 /* FIPS 180 */
#undef  SHA_1 /* FIPS 180-1 */

static char *test[]={
	"abc",
	"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
	NULL,
	};

#ifdef SHA_0
static char *ret[]={
	"0164b8a914cd2a5e74c4f7ff082c4d97f1edf880",
	"d2516ee1acfa5baf33dfc1c471e438449ef134c8",
	};
static char *bigret=
	"3232affa48628a26653b5aaa44541fd90d690603";
#endif
#ifdef SHA_1
static char *ret[]={
	"a9993e364706816aba3e25717850c26c9cd0d89d",
	"84983e441c3bd26ebaae4aa1f95129e5e54670f1",
	};
static char *bigret=
	"34aa973cd4c4daa4f61eeb2bdbad27316534016f";
#endif

static char *pt(unsigned char *md);
int main(int argc, char *argv[])
	{
	int i,err=0;
	char **P,**R;
	static unsigned char buf[1000];
	char *p,*r;
	EVP_MD_CTX c;
	unsigned char md[SHA_DIGEST_LENGTH];

#ifdef CHARSET_EBCDIC
	ebcdic2ascii(test[0], test[0], strlen(test[0]));
	ebcdic2ascii(test[1], test[1], strlen(test[1]));
#endif

	EVP_MD_CTX_init(&c);
	P=test;
	R=ret;
	i=1;
	while (*P != NULL)
		{
		EVP_Digest(*P,strlen(*P),md,NULL,EVP_sha(), NULL);
		p=pt(md);
		if (strcmp(p,*R) != 0)
			{
			printf("error calculating SHA on '%s'\n",*P);
			printf("got %s instead of %s\n",p,*R);
			err++;
			}
		else
			printf("test %d ok\n",i);
		i++;
		R++;
		P++;
		}

	memset(buf,'a',1000);
#ifdef CHARSET_EBCDIC
	ebcdic2ascii(buf, buf, 1000);
#endif /*CHARSET_EBCDIC*/
	EVP_DigestInit_ex(&c,EVP_sha(), NULL);
	for (i=0; i<1000; i++)
		EVP_DigestUpdate(&c,buf,1000);
	EVP_DigestFinal_ex(&c,md,NULL);
	p=pt(md);

	r=bigret;
	if (strcmp(p,r) != 0)
		{
		printf("error calculating SHA on '%s'\n",p);
		printf("got %s instead of %s\n",p,r);
		err++;
		}
	else
		printf("test 3 ok\n");

#ifdef OPENSSL_SYS_NETWARE
    if (err) printf("ERROR: %d\n", err);
#endif
	EVP_MD_CTX_cleanup(&c);
	EXIT(err);
	return(0);
	}

static char *pt(unsigned char *md)
	{
	int i;
	static char buf[80];

	for (i=0; i<SHA_DIGEST_LENGTH; i++)
		sprintf(&(buf[i*2]),"%02x",md[i]);
	return(buf);
	}
#endif