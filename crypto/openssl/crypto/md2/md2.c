
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
#include <openssl/md2.h>

#define BUFSIZE	1024*16

void do_fp(FILE *f);
void pt(unsigned char *md);
int read(int, void *, unsigned int);
void exit(int);
int main(int argc, char *argv[])
	{
	int i,err=0;
	FILE *IN;

	if (argc == 1)
		{
		do_fp(stdin);
		}
	else
		{
		for (i=1; i<argc; i++)
			{
			IN=fopen(argv[i],"r");
			if (IN == NULL)
				{
				perror(argv[i]);
				err++;
				continue;
				}
			printf("MD2(%s)= ",argv[i]);
			do_fp(IN);
			fclose(IN);
			}
		}
	exit(err);
	return(err);
	}

void do_fp(FILE *f)
	{
	MD2_CTX c;
	unsigned char md[MD2_DIGEST_LENGTH];
	int fd,i;
	static unsigned char buf[BUFSIZE];

	fd=fileno(f);
	MD2_Init(&c);
	for (;;)
		{
		i=read(fd,buf,BUFSIZE);
		if (i <= 0) break;
		MD2_Update(&c,buf,(unsigned long)i);
		}
	MD2_Final(&(md[0]),&c);
	pt(md);
	}

void pt(unsigned char *md)
	{
	int i;

	for (i=0; i<MD2_DIGEST_LENGTH; i++)
		printf("%02x",md[i]);
	printf("\n");
	}