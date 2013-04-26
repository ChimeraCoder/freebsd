
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
#include "apps.h"
#include <openssl/evp.h>
#include <openssl/crypto.h>
#include <openssl/bn.h>
#ifndef OPENSSL_NO_MD2
# include <openssl/md2.h>
#endif
#ifndef OPENSSL_NO_RC4
# include <openssl/rc4.h>
#endif
#ifndef OPENSSL_NO_DES
# include <openssl/des.h>
#endif
#ifndef OPENSSL_NO_IDEA
# include <openssl/idea.h>
#endif
#ifndef OPENSSL_NO_BF
# include <openssl/blowfish.h>
#endif

#undef PROG
#define PROG	version_main

int MAIN(int, char **);

int MAIN(int argc, char **argv)
	{
	int i,ret=0;
	int cflags=0,version=0,date=0,options=0,platform=0,dir=0;

	apps_startup();

	if (bio_err == NULL)
		if ((bio_err=BIO_new(BIO_s_file())) != NULL)
			BIO_set_fp(bio_err,stderr,BIO_NOCLOSE|BIO_FP_TEXT);

	if (argc == 1) version=1;
	for (i=1; i<argc; i++)
		{
		if (strcmp(argv[i],"-v") == 0)
			version=1;	
		else if (strcmp(argv[i],"-b") == 0)
			date=1;
		else if (strcmp(argv[i],"-f") == 0)
			cflags=1;
		else if (strcmp(argv[i],"-o") == 0)
			options=1;
		else if (strcmp(argv[i],"-p") == 0)
			platform=1;
		else if (strcmp(argv[i],"-d") == 0)
			dir=1;
		else if (strcmp(argv[i],"-a") == 0)
			date=version=cflags=options=platform=dir=1;
		else
			{
			BIO_printf(bio_err,"usage:version -[avbofpd]\n");
			ret=1;
			goto end;
			}
		}

	if (version)
		{
		if (SSLeay() == SSLEAY_VERSION_NUMBER)
			{
			printf("%s\n",SSLeay_version(SSLEAY_VERSION));
			}
		else
			{
			printf("%s (Library: %s)\n",
				OPENSSL_VERSION_TEXT,
				SSLeay_version(SSLEAY_VERSION));
			}
		}
	if (date)    printf("%s\n",SSLeay_version(SSLEAY_BUILT_ON));
	if (platform) printf("%s\n",SSLeay_version(SSLEAY_PLATFORM));
	if (options) 
		{
		printf("options:  ");
		printf("%s ",BN_options());
#ifndef OPENSSL_NO_MD2
		printf("%s ",MD2_options());
#endif
#ifndef OPENSSL_NO_RC4
		printf("%s ",RC4_options());
#endif
#ifndef OPENSSL_NO_DES
		printf("%s ",DES_options());
#endif
#ifndef OPENSSL_NO_IDEA
		printf("%s ",idea_options());
#endif
#ifndef OPENSSL_NO_BF
		printf("%s ",BF_options());
#endif
		printf("\n");
		}
	if (cflags)  printf("%s\n",SSLeay_version(SSLEAY_CFLAGS));
	if (dir)  printf("%s\n",SSLeay_version(SSLEAY_DIR));
end:
	apps_shutdown();
	OPENSSL_EXIT(ret);
	}