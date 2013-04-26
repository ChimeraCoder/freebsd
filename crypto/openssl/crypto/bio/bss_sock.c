
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
#include <errno.h>
#define USE_SOCKETS
#include "cryptlib.h"

#ifndef OPENSSL_NO_SOCK

#include <openssl/bio.h>

#ifdef WATT32
#define sock_write SockWrite  /* Watt-32 uses same names */
#define sock_read  SockRead
#define sock_puts  SockPuts
#endif

static int sock_write(BIO *h, const char *buf, int num);
static int sock_read(BIO *h, char *buf, int size);
static int sock_puts(BIO *h, const char *str);
static long sock_ctrl(BIO *h, int cmd, long arg1, void *arg2);
static int sock_new(BIO *h);
static int sock_free(BIO *data);
int BIO_sock_should_retry(int s);

static BIO_METHOD methods_sockp=
	{
	BIO_TYPE_SOCKET,
	"socket",
	sock_write,
	sock_read,
	sock_puts,
	NULL, /* sock_gets, */
	sock_ctrl,
	sock_new,
	sock_free,
	NULL,
	};

BIO_METHOD *BIO_s_socket(void)
	{
	return(&methods_sockp);
	}

BIO *BIO_new_socket(int fd, int close_flag)
	{
	BIO *ret;

	ret=BIO_new(BIO_s_socket());
	if (ret == NULL) return(NULL);
	BIO_set_fd(ret,fd,close_flag);
	return(ret);
	}

static int sock_new(BIO *bi)
	{
	bi->init=0;
	bi->num=0;
	bi->ptr=NULL;
	bi->flags=0;
	return(1);
	}

static int sock_free(BIO *a)
	{
	if (a == NULL) return(0);
	if (a->shutdown)
		{
		if (a->init)
			{
			SHUTDOWN2(a->num);
			}
		a->init=0;
		a->flags=0;
		}
	return(1);
	}
	
static int sock_read(BIO *b, char *out, int outl)
	{
	int ret=0;

	if (out != NULL)
		{
		clear_socket_error();
		ret=readsocket(b->num,out,outl);
		BIO_clear_retry_flags(b);
		if (ret <= 0)
			{
			if (BIO_sock_should_retry(ret))
				BIO_set_retry_read(b);
			}
		}
	return(ret);
	}

static int sock_write(BIO *b, const char *in, int inl)
	{
	int ret;
	
	clear_socket_error();
	ret=writesocket(b->num,in,inl);
	BIO_clear_retry_flags(b);
	if (ret <= 0)
		{
		if (BIO_sock_should_retry(ret))
			BIO_set_retry_write(b);
		}
	return(ret);
	}

static long sock_ctrl(BIO *b, int cmd, long num, void *ptr)
	{
	long ret=1;
	int *ip;

	switch (cmd)
		{
	case BIO_C_SET_FD:
		sock_free(b);
		b->num= *((int *)ptr);
		b->shutdown=(int)num;
		b->init=1;
		break;
	case BIO_C_GET_FD:
		if (b->init)
			{
			ip=(int *)ptr;
			if (ip != NULL) *ip=b->num;
			ret=b->num;
			}
		else
			ret= -1;
		break;
	case BIO_CTRL_GET_CLOSE:
		ret=b->shutdown;
		break;
	case BIO_CTRL_SET_CLOSE:
		b->shutdown=(int)num;
		break;
	case BIO_CTRL_DUP:
	case BIO_CTRL_FLUSH:
		ret=1;
		break;
	default:
		ret=0;
		break;
		}
	return(ret);
	}

static int sock_puts(BIO *bp, const char *str)
	{
	int n,ret;

	n=strlen(str);
	ret=sock_write(bp,str,n);
	return(ret);
	}

int BIO_sock_should_retry(int i)
	{
	int err;

	if ((i == 0) || (i == -1))
		{
		err=get_last_socket_error();

#if defined(OPENSSL_SYS_WINDOWS) && 0 /* more microsoft stupidity? perhaps not? Ben 4/1/99 */
		if ((i == -1) && (err == 0))
			return(1);
#endif

		return(BIO_sock_non_fatal_error(err));
		}
	return(0);
	}

int BIO_sock_non_fatal_error(int err)
	{
	switch (err)
		{
#if defined(OPENSSL_SYS_WINDOWS) || defined(OPENSSL_SYS_NETWARE)
# if defined(WSAEWOULDBLOCK)
	case WSAEWOULDBLOCK:
# endif

# if 0 /* This appears to always be an error */
#  if defined(WSAENOTCONN)
	case WSAENOTCONN:
#  endif
# endif
#endif

#ifdef EWOULDBLOCK
# ifdef WSAEWOULDBLOCK
#  if WSAEWOULDBLOCK != EWOULDBLOCK
	case EWOULDBLOCK:
#  endif
# else
	case EWOULDBLOCK:
# endif
#endif

#if defined(ENOTCONN)
	case ENOTCONN:
#endif

#ifdef EINTR
	case EINTR:
#endif

#ifdef EAGAIN
# if EWOULDBLOCK != EAGAIN
	case EAGAIN:
# endif
#endif

#ifdef EPROTO
	case EPROTO:
#endif

#ifdef EINPROGRESS
	case EINPROGRESS:
#endif

#ifdef EALREADY
	case EALREADY:
#endif
		return(1);
		/* break; */
	default:
		break;
		}
	return(0);
	}

#endif  /* #ifndef OPENSSL_NO_SOCK */