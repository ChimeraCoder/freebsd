
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
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>

#undef PROG
#define PROG	sess_id_main

static const char *sess_id_usage[]={
"usage: sess_id args\n",
"\n",
" -inform arg     - input format - default PEM (DER or PEM)\n",
" -outform arg    - output format - default PEM\n",
" -in arg         - input file - default stdin\n",
" -out arg        - output file - default stdout\n",
" -text           - print ssl session id details\n",
" -cert           - output certificate \n",
" -noout          - no CRL output\n",
" -context arg    - set the session ID context\n",
NULL
};

static SSL_SESSION *load_sess_id(char *file, int format);

int MAIN(int, char **);

int MAIN(int argc, char **argv)
	{
	SSL_SESSION *x=NULL;
	X509 *peer = NULL;
	int ret=1,i,num,badops=0;
	BIO *out=NULL;
	int informat,outformat;
	char *infile=NULL,*outfile=NULL,*context=NULL;
	int cert=0,noout=0,text=0;
	const char **pp;

	apps_startup();

	if (bio_err == NULL)
		if ((bio_err=BIO_new(BIO_s_file())) != NULL)
			BIO_set_fp(bio_err,stderr,BIO_NOCLOSE|BIO_FP_TEXT);

	informat=FORMAT_PEM;
	outformat=FORMAT_PEM;

	argc--;
	argv++;
	num=0;
	while (argc >= 1)
		{
		if 	(strcmp(*argv,"-inform") == 0)
			{
			if (--argc < 1) goto bad;
			informat=str2fmt(*(++argv));
			}
		else if (strcmp(*argv,"-outform") == 0)
			{
			if (--argc < 1) goto bad;
			outformat=str2fmt(*(++argv));
			}
		else if (strcmp(*argv,"-in") == 0)
			{
			if (--argc < 1) goto bad;
			infile= *(++argv);
			}
		else if (strcmp(*argv,"-out") == 0)
			{
			if (--argc < 1) goto bad;
			outfile= *(++argv);
			}
		else if (strcmp(*argv,"-text") == 0)
			text= ++num;
		else if (strcmp(*argv,"-cert") == 0)
			cert= ++num;
		else if (strcmp(*argv,"-noout") == 0)
			noout= ++num;
		else if (strcmp(*argv,"-context") == 0)
		    {
		    if(--argc < 1) goto bad;
		    context=*++argv;
		    }
		else
			{
			BIO_printf(bio_err,"unknown option %s\n",*argv);
			badops=1;
			break;
			}
		argc--;
		argv++;
		}

	if (badops)
		{
bad:
		for (pp=sess_id_usage; (*pp != NULL); pp++)
			BIO_printf(bio_err,"%s",*pp);
		goto end;
		}

	ERR_load_crypto_strings();
	x=load_sess_id(infile,informat);
	if (x == NULL) { goto end; }
	peer = SSL_SESSION_get0_peer(x);

	if(context)
	    {
	    size_t ctx_len = strlen(context);
	    if(ctx_len > SSL_MAX_SID_CTX_LENGTH)
		{
		BIO_printf(bio_err,"Context too long\n");
		goto end;
		}
	    SSL_SESSION_set1_id_context(x, (unsigned char *)context, ctx_len);
	    }

#ifdef undef
	/* just testing for memory leaks :-) */
	{
	SSL_SESSION *s;
	char buf[1024*10],*p;
	int i;

	s=SSL_SESSION_new();

	p= &buf;
	i=i2d_SSL_SESSION(x,&p);
	p= &buf;
	d2i_SSL_SESSION(&s,&p,(long)i);
	p= &buf;
	d2i_SSL_SESSION(&s,&p,(long)i);
	p= &buf;
	d2i_SSL_SESSION(&s,&p,(long)i);
	SSL_SESSION_free(s);
	}
#endif

	if (!noout || text)
		{
		out=BIO_new(BIO_s_file());
		if (out == NULL)
			{
			ERR_print_errors(bio_err);
			goto end;
			}

		if (outfile == NULL)
			{
			BIO_set_fp(out,stdout,BIO_NOCLOSE);
#ifdef OPENSSL_SYS_VMS
			{
			BIO *tmpbio = BIO_new(BIO_f_linebuffer());
			out = BIO_push(tmpbio, out);
			}
#endif
			}
		else
			{
			if (BIO_write_filename(out,outfile) <= 0)
				{
				perror(outfile);
				goto end;
				}
			}
		}

	if (text)
		{
		SSL_SESSION_print(out,x);

		if (cert)
			{
			if (peer == NULL)
				BIO_puts(out,"No certificate present\n");
			else
				X509_print(out,peer);
			}
		}

	if (!noout && !cert)
		{
		if 	(outformat == FORMAT_ASN1)
			i=i2d_SSL_SESSION_bio(out,x);
		else if (outformat == FORMAT_PEM)
			i=PEM_write_bio_SSL_SESSION(out,x);
		else	{
			BIO_printf(bio_err,"bad output format specified for outfile\n");
			goto end;
			}
		if (!i) {
			BIO_printf(bio_err,"unable to write SSL_SESSION\n");
			goto end;
			}
		}
	else if (!noout && (peer != NULL)) /* just print the certificate */
		{
		if 	(outformat == FORMAT_ASN1)
			i=(int)i2d_X509_bio(out,peer);
		else if (outformat == FORMAT_PEM)
			i=PEM_write_bio_X509(out,peer);
		else	{
			BIO_printf(bio_err,"bad output format specified for outfile\n");
			goto end;
			}
		if (!i) {
			BIO_printf(bio_err,"unable to write X509\n");
			goto end;
			}
		}
	ret=0;
end:
	if (out != NULL) BIO_free_all(out);
	if (x != NULL) SSL_SESSION_free(x);
	apps_shutdown();
	OPENSSL_EXIT(ret);
	}

static SSL_SESSION *load_sess_id(char *infile, int format)
	{
	SSL_SESSION *x=NULL;
	BIO *in=NULL;

	in=BIO_new(BIO_s_file());
	if (in == NULL)
		{
		ERR_print_errors(bio_err);
		goto end;
		}

	if (infile == NULL)
		BIO_set_fp(in,stdin,BIO_NOCLOSE);
	else
		{
		if (BIO_read_filename(in,infile) <= 0)
			{
			perror(infile);
			goto end;
			}
		}
	if 	(format == FORMAT_ASN1)
		x=d2i_SSL_SESSION_bio(in,NULL);
	else if (format == FORMAT_PEM)
		x=PEM_read_bio_SSL_SESSION(in,NULL,NULL,NULL);
	else	{
		BIO_printf(bio_err,"bad input format specified for input crl\n");
		goto end;
		}
	if (x == NULL)
		{
		BIO_printf(bio_err,"unable to load SSL_SESSION\n");
		ERR_print_errors(bio_err);
		goto end;
		}
	
end:
	if (in != NULL) BIO_free(in);
	return(x);
	}