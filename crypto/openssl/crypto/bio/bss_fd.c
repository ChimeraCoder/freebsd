
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

#if defined(OPENSSL_NO_POSIX_IO)
/*
 * One can argue that one should implement dummy placeholder for
 * BIO_s_fd here...
 */
#else
/*
 * As for unconditional usage of "UPLINK" interface in this module.
 * Trouble is that unlike Unix file descriptors [which are indexes
 * in kernel-side per-process table], corresponding descriptors on
 * platforms which require "UPLINK" interface seem to be indexes
 * in a user-land, non-global table. Well, in fact they are indexes
 * in stdio _iob[], and recall that _iob[] was the very reason why
 * "UPLINK" interface was introduced in first place. But one way on
 * another. Neither libcrypto or libssl use this BIO meaning that
 * file descriptors can only be provided by application. Therefore
 * "UPLINK" calls are due...
 */
#include "bio_lcl.h"

static int fd_write(BIO *h, const char *buf, int num);
static int fd_read(BIO *h, char *buf, int size);
static int fd_puts(BIO *h, const char *str);
static int fd_gets(BIO *h, char *buf, int size);
static long fd_ctrl(BIO *h, int cmd, long arg1, void *arg2);
static int fd_new(BIO *h);
static int fd_free(BIO *data);
int BIO_fd_should_retry(int s);

static BIO_METHOD methods_fdp=
	{
	BIO_TYPE_FD,"file descriptor",
	fd_write,
	fd_read,
	fd_puts,
	fd_gets,
	fd_ctrl,
	fd_new,
	fd_free,
	NULL,
	};

BIO_METHOD *BIO_s_fd(void)
	{
	return(&methods_fdp);
	}

BIO *BIO_new_fd(int fd,int close_flag)
	{
	BIO *ret;
	ret=BIO_new(BIO_s_fd());
	if (ret == NULL) return(NULL);
	BIO_set_fd(ret,fd,close_flag);
	return(ret);
	}

static int fd_new(BIO *bi)
	{
	bi->init=0;
	bi->num=-1;
	bi->ptr=NULL;
	bi->flags=BIO_FLAGS_UPLINK; /* essentially redundant */
	return(1);
	}

static int fd_free(BIO *a)
	{
	if (a == NULL) return(0);
	if (a->shutdown)
		{
		if (a->init)
			{
			UP_close(a->num);
			}
		a->init=0;
		a->flags=BIO_FLAGS_UPLINK;
		}
	return(1);
	}
	
static int fd_read(BIO *b, char *out,int outl)
	{
	int ret=0;

	if (out != NULL)
		{
		clear_sys_error();
		ret=UP_read(b->num,out,outl);
		BIO_clear_retry_flags(b);
		if (ret <= 0)
			{
			if (BIO_fd_should_retry(ret))
				BIO_set_retry_read(b);
			}
		}
	return(ret);
	}

static int fd_write(BIO *b, const char *in, int inl)
	{
	int ret;
	clear_sys_error();
	ret=UP_write(b->num,in,inl);
	BIO_clear_retry_flags(b);
	if (ret <= 0)
		{
		if (BIO_fd_should_retry(ret))
			BIO_set_retry_write(b);
		}
	return(ret);
	}

static long fd_ctrl(BIO *b, int cmd, long num, void *ptr)
	{
	long ret=1;
	int *ip;

	switch (cmd)
		{
	case BIO_CTRL_RESET:
		num=0;
	case BIO_C_FILE_SEEK:
		ret=(long)UP_lseek(b->num,num,0);
		break;
	case BIO_C_FILE_TELL:
	case BIO_CTRL_INFO:
		ret=(long)UP_lseek(b->num,0,1);
		break;
	case BIO_C_SET_FD:
		fd_free(b);
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
	case BIO_CTRL_PENDING:
	case BIO_CTRL_WPENDING:
		ret=0;
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

static int fd_puts(BIO *bp, const char *str)
	{
	int n,ret;

	n=strlen(str);
	ret=fd_write(bp,str,n);
	return(ret);
	}

static int fd_gets(BIO *bp, char *buf, int size)
        {
	int ret=0;
	char *ptr=buf;
	char *end=buf+size-1;

	while ( (ptr < end) && (fd_read(bp, ptr, 1) > 0) && (ptr[0] != '\n') )
		ptr++;

	ptr[0]='\0';

	if (buf[0] != '\0')
		ret=strlen(buf);
	return(ret);
        }

int BIO_fd_should_retry(int i)
	{
	int err;

	if ((i == 0) || (i == -1))
		{
		err=get_last_sys_error();

#if defined(OPENSSL_SYS_WINDOWS) && 0 /* more microsoft stupidity? perhaps not? Ben 4/1/99 */
		if ((i == -1) && (err == 0))
			return(1);
#endif

		return(BIO_fd_non_fatal_error(err));
		}
	return(0);
	}

int BIO_fd_non_fatal_error(int err)
	{
	switch (err)
		{

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
#if EWOULDBLOCK != EAGAIN
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
#endif