
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
#include "cryptlib.h"
#include <openssl/bio.h>
#include <openssl/err.h>

long MS_CALLBACK BIO_debug_callback(BIO *bio, int cmd, const char *argp,
	     int argi, long argl, long ret)
	{
	BIO *b;
	MS_STATIC char buf[256];
	char *p;
	long r=1;
	size_t p_maxlen;

	if (BIO_CB_RETURN & cmd)
		r=ret;

	BIO_snprintf(buf,sizeof buf,"BIO[%08lX]:",(unsigned long)bio);
	p= &(buf[14]);
	p_maxlen = sizeof buf - 14;
	switch (cmd)
		{
	case BIO_CB_FREE:
		BIO_snprintf(p,p_maxlen,"Free - %s\n",bio->method->name);
		break;
	case BIO_CB_READ:
		if (bio->method->type & BIO_TYPE_DESCRIPTOR)
			BIO_snprintf(p,p_maxlen,"read(%d,%lu) - %s fd=%d\n",
				 bio->num,(unsigned long)argi,
				 bio->method->name,bio->num);
		else
			BIO_snprintf(p,p_maxlen,"read(%d,%lu) - %s\n",
				 bio->num,(unsigned long)argi,
				 bio->method->name);
		break;
	case BIO_CB_WRITE:
		if (bio->method->type & BIO_TYPE_DESCRIPTOR)
			BIO_snprintf(p,p_maxlen,"write(%d,%lu) - %s fd=%d\n",
				 bio->num,(unsigned long)argi,
				 bio->method->name,bio->num);
		else
			BIO_snprintf(p,p_maxlen,"write(%d,%lu) - %s\n",
				 bio->num,(unsigned long)argi,
				 bio->method->name);
		break;
	case BIO_CB_PUTS:
		BIO_snprintf(p,p_maxlen,"puts() - %s\n",bio->method->name);
		break;
	case BIO_CB_GETS:
		BIO_snprintf(p,p_maxlen,"gets(%lu) - %s\n",(unsigned long)argi,bio->method->name);
		break;
	case BIO_CB_CTRL:
		BIO_snprintf(p,p_maxlen,"ctrl(%lu) - %s\n",(unsigned long)argi,bio->method->name);
		break;
	case BIO_CB_RETURN|BIO_CB_READ:
		BIO_snprintf(p,p_maxlen,"read return %ld\n",ret);
		break;
	case BIO_CB_RETURN|BIO_CB_WRITE:
		BIO_snprintf(p,p_maxlen,"write return %ld\n",ret);
		break;
	case BIO_CB_RETURN|BIO_CB_GETS:
		BIO_snprintf(p,p_maxlen,"gets return %ld\n",ret);
		break;
	case BIO_CB_RETURN|BIO_CB_PUTS:
		BIO_snprintf(p,p_maxlen,"puts return %ld\n",ret);
		break;
	case BIO_CB_RETURN|BIO_CB_CTRL:
		BIO_snprintf(p,p_maxlen,"ctrl return %ld\n",ret);
		break;
	default:
		BIO_snprintf(p,p_maxlen,"bio callback - unknown type (%d)\n",cmd);
		break;
		}

	b=(BIO *)bio->cb_arg;
	if (b != NULL)
		BIO_write(b,buf,strlen(buf));
#if !defined(OPENSSL_NO_STDIO) && !defined(OPENSSL_SYS_WIN16)
	else
		fputs(buf,stderr);
#endif
	return(r);
	}