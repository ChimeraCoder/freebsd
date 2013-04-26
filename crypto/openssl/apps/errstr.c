
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
#include <openssl/bio.h>
#include <openssl/lhash.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#undef PROG
#define PROG	errstr_main

int MAIN(int, char **);

int MAIN(int argc, char **argv)
	{
	int i,ret=0;
	char buf[256];
	unsigned long l;

	apps_startup();

	if (bio_err == NULL)
		if ((bio_err=BIO_new(BIO_s_file())) != NULL)
			BIO_set_fp(bio_err,stderr,BIO_NOCLOSE|BIO_FP_TEXT);

	SSL_load_error_strings();

	if ((argc > 1) && (strcmp(argv[1],"-stats") == 0))
		{
		BIO *out=NULL;

		out=BIO_new(BIO_s_file());
		if ((out != NULL) && BIO_set_fp(out,stdout,BIO_NOCLOSE))
			{
#ifdef OPENSSL_SYS_VMS
			{
			BIO *tmpbio = BIO_new(BIO_f_linebuffer());
			out = BIO_push(tmpbio, out);
			}
#endif
			lh_ERR_STRING_DATA_node_stats_bio(
						  ERR_get_string_table(), out);
			lh_ERR_STRING_DATA_stats_bio(ERR_get_string_table(),
						     out);
			lh_ERR_STRING_DATA_node_usage_stats_bio(
						    ERR_get_string_table(),out);
			}
		if (out != NULL) BIO_free_all(out);
		argc--;
		argv++;
		}

	for (i=1; i<argc; i++)
		{
		if (sscanf(argv[i],"%lx",&l))
			{
			ERR_error_string_n(l, buf, sizeof buf);
			printf("%s\n",buf);
			}
		else
			{
			printf("%s: bad error code\n",argv[i]);
			printf("usage: errstr [-stats] <errno> ...\n");
			ret++;
			}
		}
	apps_shutdown();
	OPENSSL_EXIT(ret);
	}