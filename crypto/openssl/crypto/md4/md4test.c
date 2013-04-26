
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

#ifdef OPENSSL_NO_MD4
int main(int argc, char *argv[])
{
    printf("No MD4 support\n");
    return(0);
}
#else
#include <openssl/evp.h>
#include <openssl/md4.h>

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
"31d6cfe0d16ae931b73c59d7e0c089c0",
"bde52cb31de33e46245e05fbdbd6fb24",
"a448017aaf21d8525fc10ae87aa6729d",
"d9130a8164549fe818874806e1c7014b",
"d79e1c308aa5bbcdeea8ed63df412da9",
"043f8582f241db351ce627e153e7f0e4",
"e33b4ddc9c38f2199c3e7b164fcc0536",
};

static char *pt(unsigned char *md);
int main(int argc, char *argv[])
	{
	int i,err=0;
	char **P,**R;
	char *p;
	unsigned char md[MD4_DIGEST_LENGTH];

	P=test;
	R=ret;
	i=1;
	while (*P != NULL)
		{
		EVP_Digest(&(P[0][0]),strlen((char *)*P),md,NULL,EVP_md4(), NULL);
		p=pt(md);
		if (strcmp(p,(char *)*R) != 0)
			{
			printf("error calculating MD4 on '%s'\n",*P);
			printf("got %s instead of %s\n",p,*R);
			err++;
			}
		else
			printf("test %d ok\n",i);
		i++;
		R++;
		P++;
		}
	EXIT(err);
	return(0);
	}

static char *pt(unsigned char *md)
	{
	int i;
	static char buf[80];

	for (i=0; i<MD4_DIGEST_LENGTH; i++)
		sprintf(&(buf[i*2]),"%02x",md[i]);
	return(buf);
	}
#endif