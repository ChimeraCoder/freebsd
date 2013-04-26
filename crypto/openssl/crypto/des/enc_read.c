
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
#include "des_locl.h"

/* This has some uglies in it but it works - even over sockets. */
/*extern int errno;*/
OPENSSL_IMPLEMENT_GLOBAL(int,DES_rw_mode,DES_PCBC_MODE)


/*
 * WARNINGS:
 *
 *  -  The data format used by DES_enc_write() and DES_enc_read()
 *     has a cryptographic weakness: When asked to write more
 *     than MAXWRITE bytes, DES_enc_write will split the data
 *     into several chunks that are all encrypted
 *     using the same IV.  So don't use these functions unless you
 *     are sure you know what you do (in which case you might
 *     not want to use them anyway).
 *
 *  -  This code cannot handle non-blocking sockets.
 *
 *  -  This function uses an internal state and thus cannot be
 *     used on multiple files.
 */


int DES_enc_read(int fd, void *buf, int len, DES_key_schedule *sched,
		 DES_cblock *iv)
	{
#if defined(OPENSSL_NO_POSIX_IO)
	return(0);
#else
	/* data to be unencrypted */
	int net_num=0;
	static unsigned char *net=NULL;
	/* extra unencrypted data 
	 * for when a block of 100 comes in but is des_read one byte at
	 * a time. */
	static unsigned char *unnet=NULL;
	static int unnet_start=0;
	static int unnet_left=0;
	static unsigned char *tmpbuf=NULL;
	int i;
	long num=0,rnum;
	unsigned char *p;

	if (tmpbuf == NULL)
		{
		tmpbuf=OPENSSL_malloc(BSIZE);
		if (tmpbuf == NULL) return(-1);
		}
	if (net == NULL)
		{
		net=OPENSSL_malloc(BSIZE);
		if (net == NULL) return(-1);
		}
	if (unnet == NULL)
		{
		unnet=OPENSSL_malloc(BSIZE);
		if (unnet == NULL) return(-1);
		}
	/* left over data from last decrypt */
	if (unnet_left != 0)
		{
		if (unnet_left < len)
			{
			/* we still still need more data but will return
			 * with the number of bytes we have - should always
			 * check the return value */
			memcpy(buf,&(unnet[unnet_start]),
			       unnet_left);
			/* eay 26/08/92 I had the next 2 lines
			 * reversed :-( */
			i=unnet_left;
			unnet_start=unnet_left=0;
			}
		else
			{
			memcpy(buf,&(unnet[unnet_start]),len);
			unnet_start+=len;
			unnet_left-=len;
			i=len;
			}
		return(i);
		}

	/* We need to get more data. */
	if (len > MAXWRITE) len=MAXWRITE;

	/* first - get the length */
	while (net_num < HDRSIZE) 
		{
#ifndef OPENSSL_SYS_WIN32
		i=read(fd,(void *)&(net[net_num]),HDRSIZE-net_num);
#else
		i=_read(fd,(void *)&(net[net_num]),HDRSIZE-net_num);
#endif
#ifdef EINTR
		if ((i == -1) && (errno == EINTR)) continue;
#endif
		if (i <= 0) return(0);
		net_num+=i;
		}

	/* we now have at net_num bytes in net */
	p=net;
	/* num=0;  */
	n2l(p,num);
	/* num should be rounded up to the next group of eight
	 * we make sure that we have read a multiple of 8 bytes from the net.
	 */
	if ((num > MAXWRITE) || (num < 0)) /* error */
		return(-1);
	rnum=(num < 8)?8:((num+7)/8*8);

	net_num=0;
	while (net_num < rnum)
		{
#ifndef OPENSSL_SYS_WIN32
		i=read(fd,(void *)&(net[net_num]),rnum-net_num);
#else
		i=_read(fd,(void *)&(net[net_num]),rnum-net_num);
#endif
#ifdef EINTR
		if ((i == -1) && (errno == EINTR)) continue;
#endif
		if (i <= 0) return(0);
		net_num+=i;
		}

	/* Check if there will be data left over. */
	if (len < num)
		{
		if (DES_rw_mode & DES_PCBC_MODE)
			DES_pcbc_encrypt(net,unnet,num,sched,iv,DES_DECRYPT);
		else
			DES_cbc_encrypt(net,unnet,num,sched,iv,DES_DECRYPT);
		memcpy(buf,unnet,len);
		unnet_start=len;
		unnet_left=num-len;

		/* The following line is done because we return num
		 * as the number of bytes read. */
		num=len;
		}
	else
		{
		/* >output is a multiple of 8 byes, if len < rnum
		 * >we must be careful.  The user must be aware that this
		 * >routine will write more bytes than he asked for.
		 * >The length of the buffer must be correct.
		 * FIXED - Should be ok now 18-9-90 - eay */
		if (len < rnum)
			{

			if (DES_rw_mode & DES_PCBC_MODE)
				DES_pcbc_encrypt(net,tmpbuf,num,sched,iv,
						 DES_DECRYPT);
			else
				DES_cbc_encrypt(net,tmpbuf,num,sched,iv,
						DES_DECRYPT);

			/* eay 26/08/92 fix a bug that returned more
			 * bytes than you asked for (returned len bytes :-( */
			memcpy(buf,tmpbuf,num);
			}
		else
			{
			if (DES_rw_mode & DES_PCBC_MODE)
				DES_pcbc_encrypt(net,buf,num,sched,iv,
						 DES_DECRYPT);
			else
				DES_cbc_encrypt(net,buf,num,sched,iv,
						DES_DECRYPT);
			}
		}
	return num;
#endif /* OPENSSL_NO_POSIX_IO */
	}