
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
#include "cryptlib.h"
#include <openssl/rand.h>
#include <openssl/bio.h>

/* BIO_put and BIO_get both add to the digest,
 * BIO_gets returns the digest */

static int nbiof_write(BIO *h,const char *buf,int num);
static int nbiof_read(BIO *h,char *buf,int size);
static int nbiof_puts(BIO *h,const char *str);
static int nbiof_gets(BIO *h,char *str,int size);
static long nbiof_ctrl(BIO *h,int cmd,long arg1,void *arg2);
static int nbiof_new(BIO *h);
static int nbiof_free(BIO *data);
static long nbiof_callback_ctrl(BIO *h,int cmd,bio_info_cb *fp);
typedef struct nbio_test_st
	{
	/* only set if we sent a 'should retry' error */
	int lrn;
	int lwn;
	} NBIO_TEST;

static BIO_METHOD methods_nbiof=
	{
	BIO_TYPE_NBIO_TEST,
	"non-blocking IO test filter",
	nbiof_write,
	nbiof_read,
	nbiof_puts,
	nbiof_gets,
	nbiof_ctrl,
	nbiof_new,
	nbiof_free,
	nbiof_callback_ctrl,
	};

BIO_METHOD *BIO_f_nbio_test(void)
	{
	return(&methods_nbiof);
	}

static int nbiof_new(BIO *bi)
	{
	NBIO_TEST *nt;

	if (!(nt=(NBIO_TEST *)OPENSSL_malloc(sizeof(NBIO_TEST)))) return(0);
	nt->lrn= -1;
	nt->lwn= -1;
	bi->ptr=(char *)nt;
	bi->init=1;
	bi->flags=0;
	return(1);
	}

static int nbiof_free(BIO *a)
	{
	if (a == NULL) return(0);
	if (a->ptr != NULL)
		OPENSSL_free(a->ptr);
	a->ptr=NULL;
	a->init=0;
	a->flags=0;
	return(1);
	}
	
static int nbiof_read(BIO *b, char *out, int outl)
	{
	int ret=0;
#if 1
	int num;
	unsigned char n;
#endif

	if (out == NULL) return(0);
	if (b->next_bio == NULL) return(0);

	BIO_clear_retry_flags(b);
#if 1
	RAND_pseudo_bytes(&n,1);
	num=(n&0x07);

	if (outl > num) outl=num;

	if (num == 0)
		{
		ret= -1;
		BIO_set_retry_read(b);
		}
	else
#endif
		{
		ret=BIO_read(b->next_bio,out,outl);
		if (ret < 0)
			BIO_copy_next_retry(b);
		}
	return(ret);
	}

static int nbiof_write(BIO *b, const char *in, int inl)
	{
	NBIO_TEST *nt;
	int ret=0;
	int num;
	unsigned char n;

	if ((in == NULL) || (inl <= 0)) return(0);
	if (b->next_bio == NULL) return(0);
	nt=(NBIO_TEST *)b->ptr;

	BIO_clear_retry_flags(b);

#if 1
	if (nt->lwn > 0)
		{
		num=nt->lwn;
		nt->lwn=0;
		}
	else
		{
		RAND_pseudo_bytes(&n,1);
		num=(n&7);
		}

	if (inl > num) inl=num;

	if (num == 0)
		{
		ret= -1;
		BIO_set_retry_write(b);
		}
	else
#endif
		{
		ret=BIO_write(b->next_bio,in,inl);
		if (ret < 0)
			{
			BIO_copy_next_retry(b);
			nt->lwn=inl;
			}
		}
	return(ret);
	}

static long nbiof_ctrl(BIO *b, int cmd, long num, void *ptr)
	{
	long ret;

	if (b->next_bio == NULL) return(0);
	switch (cmd)
		{
        case BIO_C_DO_STATE_MACHINE:
		BIO_clear_retry_flags(b);
		ret=BIO_ctrl(b->next_bio,cmd,num,ptr);
		BIO_copy_next_retry(b);
		break;
	case BIO_CTRL_DUP:
		ret=0L;
		break;
	default:
		ret=BIO_ctrl(b->next_bio,cmd,num,ptr);
		break;
		}
	return(ret);
	}

static long nbiof_callback_ctrl(BIO *b, int cmd, bio_info_cb *fp)
	{
	long ret=1;

	if (b->next_bio == NULL) return(0);
	switch (cmd)
		{
	default:
		ret=BIO_callback_ctrl(b->next_bio,cmd,fp);
		break;
		}
	return(ret);
	}

static int nbiof_gets(BIO *bp, char *buf, int size)
	{
	if (bp->next_bio == NULL) return(0);
	return(BIO_gets(bp->next_bio,buf,size));
	}


static int nbiof_puts(BIO *bp, const char *str)
	{
	if (bp->next_bio == NULL) return(0);
	return(BIO_puts(bp->next_bio,str));
	}