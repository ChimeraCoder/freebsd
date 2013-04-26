
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

#ifdef OPENSSL_NO_MD5
int main(int argc, char *argv[])
{
    printf("No MD5 support\n");
    return(0);
}
#else
#include <openssl/evp.h>
#include <openssl/md5.h>

static char *test[]={
	"",
	"a",
	"abc",
	"message digest",
	"abcdefghijklmnopqrstuvwxyz",
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
	"12345678901234567890123456789012345678901234567890123456789012345678901234567890",
	NULL,
	};

static char *ret[]={
	"d41d8cd98f00b204e9800998ecf8427e",
	"0cc175b9c0f1b6a831c399e269772661",
	"900150983cd24fb0d6963f7d28e17f72",
	"f96b697d7cb7938d525a2f31aaf161d0",
	"c3fcd3d76192e4007dfb496cca67e13b",
	"d174ab98d277d9f5a5611c2c9f419d9f",
	"57edf4a22be3c955ac49da2e2107b67a",
	};

static char *pt(unsigned char *md);
int main(int argc, char *argv[])
	{
	int i,err=0;
	char **P,**R;
	char *p;
	unsigned char md[MD5_DIGEST_LENGTH];

	P=test;
	R=ret;
	i=1;
	while (*P != NULL)
		{
		EVP_Digest(&(P[0][0]),strlen((char *)*P),md,NULL,EVP_md5(), NULL);
		p=pt(md);
		if (strcmp(p,(char *)*R) != 0)
			{
			printf("error calculating MD5 on '%s'\n",*P);
			printf("got %s instead of %s\n",p,*R);
			err++;
			}
		else
			printf("test %d ok\n",i);
		i++;
		R++;
		P++;
		}

#ifdef OPENSSL_SYS_NETWARE
    if (err) printf("ERROR: %d\n", err);
#endif
	EXIT(err);
	return(0);
	}

static char *pt(unsigned char *md)
	{
	int i;
	static char buf[80];

	for (i=0; i<MD5_DIGEST_LENGTH; i++)
		sprintf(&(buf[i*2]),"%02x",md[i]);
	return(buf);
	}
#endif