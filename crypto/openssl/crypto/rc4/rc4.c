
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
#include <openssl/rc4.h>
#include <openssl/evp.h>

char *usage[]={
"usage: rc4 args\n",
"\n",
" -in arg         - input file - default stdin\n",
" -out arg        - output file - default stdout\n",
" -key key        - password\n",
NULL
};

int main(int argc, char *argv[])
	{
	FILE *in=NULL,*out=NULL;
	char *infile=NULL,*outfile=NULL,*keystr=NULL;
	RC4_KEY key;
	char buf[BUFSIZ];
	int badops=0,i;
	char **pp;
	unsigned char md[MD5_DIGEST_LENGTH];

	argc--;
	argv++;
	while (argc >= 1)
		{
		if 	(strcmp(*argv,"-in") == 0)
			{
			if (--argc < 1) goto bad;
			infile= *(++argv);
			}
		else if (strcmp(*argv,"-out") == 0)
			{
			if (--argc < 1) goto bad;
			outfile= *(++argv);
			}
		else if (strcmp(*argv,"-key") == 0)
			{
			if (--argc < 1) goto bad;
			keystr= *(++argv);
			}
		else
			{
			fprintf(stderr,"unknown option %s\n",*argv);
			badops=1;
			break;
			}
		argc--;
		argv++;
		}

	if (badops)
		{
bad:
		for (pp=usage; (*pp != NULL); pp++)
			fprintf(stderr,"%s",*pp);
		exit(1);
		}

	if (infile == NULL)
		in=stdin;
	else
		{
		in=fopen(infile,"r");
		if (in == NULL)
			{
			perror("open");
			exit(1);
			}

		}
	if (outfile == NULL)
		out=stdout;
	else
		{
		out=fopen(outfile,"w");
		if (out == NULL)
			{
			perror("open");
			exit(1);
			}
		}
		
#ifdef OPENSSL_SYS_MSDOS
	/* This should set the file to binary mode. */
	{
#include <fcntl.h>
	setmode(fileno(in),O_BINARY);
	setmode(fileno(out),O_BINARY);
	}
#endif

	if (keystr == NULL)
		{ /* get key */
		i=EVP_read_pw_string(buf,BUFSIZ,"Enter RC4 password:",0);
		if (i != 0)
			{
			OPENSSL_cleanse(buf,BUFSIZ);
			fprintf(stderr,"bad password read\n");
			exit(1);
			}
		keystr=buf;
		}

	EVP_Digest((unsigned char *)keystr,strlen(keystr),md,NULL,EVP_md5(),NULL);
	OPENSSL_cleanse(keystr,strlen(keystr));
	RC4_set_key(&key,MD5_DIGEST_LENGTH,md);
	
	for(;;)
		{
		i=fread(buf,1,BUFSIZ,in);
		if (i == 0) break;
		if (i < 0)
			{
			perror("read");
			exit(1);
			}
		RC4(&key,(unsigned int)i,(unsigned char *)buf,
			(unsigned char *)buf);
		i=fwrite(buf,(unsigned int)i,1,out);
		if (i != 1)
			{
			perror("write");
			exit(1);
			}
		}
	fclose(out);
	fclose(in);
	exit(0);
	return(1);
	}