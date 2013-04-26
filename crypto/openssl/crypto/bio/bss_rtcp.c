
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

/* Written by David L. Jones <jonesd@kcgl1.eng.ohio-state.edu>
 * Date:   22-JUL-1996
 * Revised: 25-SEP-1997		Update for 0.8.1, BIO_CTRL_SET -> BIO_C_SET_FD
 */
/* VMS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "cryptlib.h"
#include <openssl/bio.h>

#include <iodef.h>		/* VMS IO$_ definitions */
#include <starlet.h>

typedef unsigned short io_channel;
/*************************************************************************/
struct io_status { short status, count; long flags; };

struct rpc_msg {		/* Should have member alignment inhibited */
   char channel;		/* 'A'-app data. 'R'-remote client 'G'-global */
   char function;		/* 'G'-get, 'P'-put, 'C'-confirm, 'X'-close */
   unsigned short int length;	/* Amount of data returned or max to return */
   char data[4092];		/* variable data */
};
#define RPC_HDR_SIZE (sizeof(struct rpc_msg) - 4092)

struct rpc_ctx {
    int filled, pos;
    struct rpc_msg msg;
};

static int rtcp_write(BIO *h,const char *buf,int num);
static int rtcp_read(BIO *h,char *buf,int size);
static int rtcp_puts(BIO *h,const char *str);
static int rtcp_gets(BIO *h,char *str,int size);
static long rtcp_ctrl(BIO *h,int cmd,long arg1,void *arg2);
static int rtcp_new(BIO *h);
static int rtcp_free(BIO *data);

static BIO_METHOD rtcp_method=
	{
	BIO_TYPE_FD,
	"RTCP",
	rtcp_write,
	rtcp_read,
	rtcp_puts,
	rtcp_gets,
	rtcp_ctrl,
	rtcp_new,
	rtcp_free,
	NULL,
	};

BIO_METHOD *BIO_s_rtcp(void)
	{
	return(&rtcp_method);
	}
/*****************************************************************************/
/* Decnet I/O routines.
 */

#ifdef __DECC
#pragma message save
#pragma message disable DOLLARID
#endif

static int get ( io_channel chan, char *buffer, int maxlen, int *length )
{
    int status;
    struct io_status iosb;
    status = sys$qiow ( 0, chan, IO$_READVBLK, &iosb, 0, 0,
	buffer, maxlen, 0, 0, 0, 0 );
    if ( (status&1) == 1 ) status = iosb.status;
    if ( (status&1) == 1 ) *length = iosb.count;
    return status;
}

static int put ( io_channel chan, char *buffer, int length )
{
    int status;
    struct io_status iosb;
    status = sys$qiow ( 0, chan, IO$_WRITEVBLK, &iosb, 0, 0,
	buffer, length, 0, 0, 0, 0 );
    if ( (status&1) == 1 ) status = iosb.status;
    return status;
}

#ifdef __DECC
#pragma message restore
#endif

/***************************************************************************/

static int rtcp_new(BIO *bi)
{
    struct rpc_ctx *ctx;
	bi->init=1;
	bi->num=0;
	bi->flags = 0;
	bi->ptr=OPENSSL_malloc(sizeof(struct rpc_ctx));
	ctx = (struct rpc_ctx *) bi->ptr;
	ctx->filled = 0;
	ctx->pos = 0;
	return(1);
}

static int rtcp_free(BIO *a)
{
	if (a == NULL) return(0);
	if ( a->ptr ) OPENSSL_free ( a->ptr );
	a->ptr = NULL;
	return(1);
}
	
static int rtcp_read(BIO *b, char *out, int outl)
{
    int status, length;
    struct rpc_ctx *ctx;
    /*
     * read data, return existing.
     */
    ctx = (struct rpc_ctx *) b->ptr;
    if ( ctx->pos < ctx->filled ) {
	length = ctx->filled - ctx->pos;
	if ( length > outl ) length = outl;
	memmove ( out, &ctx->msg.data[ctx->pos], length );
	ctx->pos += length;
	return length;
    }
    /*
     * Requst more data from R channel.
     */
    ctx->msg.channel = 'R';
    ctx->msg.function = 'G';
    ctx->msg.length = sizeof(ctx->msg.data);
    status = put ( b->num, (char *) &ctx->msg, RPC_HDR_SIZE );
    if ( (status&1) == 0 ) {
	return -1;
    }
    /*
     * Read.
     */
    ctx->pos = ctx->filled = 0;
    status = get ( b->num, (char *) &ctx->msg, sizeof(ctx->msg), &length );
    if ( (status&1) == 0 ) length = -1;
    if ( ctx->msg.channel != 'R' || ctx->msg.function != 'C' ) {
	length = -1;
    }
    ctx->filled = length - RPC_HDR_SIZE;
    
    if ( ctx->pos < ctx->filled ) {
	length = ctx->filled - ctx->pos;
	if ( length > outl ) length = outl;
	memmove ( out, ctx->msg.data, length );
	ctx->pos += length;
	return length;
    }

    return length;
}

static int rtcp_write(BIO *b, const char *in, int inl)
{
    int status, i, segment, length;
    struct rpc_ctx *ctx;
    /*
     * Output data, send in chunks no larger that sizeof(ctx->msg.data).
     */
    ctx = (struct rpc_ctx *) b->ptr;
    for ( i = 0; i < inl; i += segment ) {
	segment = inl - i;
	if ( segment > sizeof(ctx->msg.data) ) segment = sizeof(ctx->msg.data);
	ctx->msg.channel = 'R';
	ctx->msg.function = 'P';
	ctx->msg.length = segment;
	memmove ( ctx->msg.data, &in[i], segment );
	status = put ( b->num, (char *) &ctx->msg, segment + RPC_HDR_SIZE );
	if ((status&1) == 0 ) { i = -1; break; }

	status = get ( b->num, (char *) &ctx->msg, sizeof(ctx->msg), &length );
	if ( ((status&1) == 0) || (length < RPC_HDR_SIZE) ) { i = -1; break; }
	if ( (ctx->msg.channel != 'R') || (ctx->msg.function != 'C') ) {
	   printf("unexpected response when confirming put %c %c\n",
		ctx->msg.channel, ctx->msg.function );

	}
    }
    return(i);
}

static long rtcp_ctrl(BIO *b, int cmd, long num, void *ptr)
	{
	long ret=1;

	switch (cmd)
		{
	case BIO_CTRL_RESET:
	case BIO_CTRL_EOF:
		ret = 1;
		break;
	case BIO_C_SET_FD:
		b->num = num;
		ret = 1;
	 	break;
	case BIO_CTRL_SET_CLOSE:
	case BIO_CTRL_FLUSH:
	case BIO_CTRL_DUP:
		ret=1;
		break;
	case BIO_CTRL_GET_CLOSE:
	case BIO_CTRL_INFO:
	case BIO_CTRL_GET:
	case BIO_CTRL_PENDING:
	case BIO_CTRL_WPENDING:
	default:
		ret=0;
		break;
		}
	return(ret);
	}

static int rtcp_gets(BIO *bp, char *buf, int size)
	{
	return(0);
	}

static int rtcp_puts(BIO *bp, const char *str)
{
    int length;
    if (str == NULL) return(0);
    length = strlen ( str );
    if ( length == 0 ) return (0);
    return rtcp_write ( bp,str, length );
}