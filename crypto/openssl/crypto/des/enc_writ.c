
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

#include <errno.h>
#include <time.h>
#include <stdio.h>
#include "cryptlib.h"
#include "des_locl.h"
#include <openssl/rand.h>

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
 */

int DES_enc_write(int fd, const void *_buf, int len,
		  DES_key_schedule *sched, DES_cblock *iv)
	{
#if defined(OPENSSL_NO_POSIX_IO)
	return (-1);
#else
#ifdef _LIBC
	extern unsigned long time();
	extern int write();
#endif
	const unsigned char *buf=_buf;
	long rnum;
	int i,j,k,outnum;
	static unsigned char *outbuf=NULL;
	unsigned char shortbuf[8];
	unsigned char *p;
	const unsigned char *cp;
	static int start=1;

	if (outbuf == NULL)
		{
		outbuf=OPENSSL_malloc(BSIZE+HDRSIZE);
		if (outbuf == NULL) return(-1);
		}
	/* If we are sending less than 8 bytes, the same char will look
	 * the same if we don't pad it out with random bytes */
	if (start)
		{
		start=0;
		}

	/* lets recurse if we want to send the data in small chunks */
	if (len > MAXWRITE)
		{
		j=0;
		for (i=0; i<len; i+=k)
			{
			k=DES_enc_write(fd,&(buf[i]),
				((len-i) > MAXWRITE)?MAXWRITE:(len-i),sched,iv);
			if (k < 0)
				return(k);
			else
				j+=k;
			}
		return(j);
		}

	/* write length first */
	p=outbuf;
	l2n(len,p);

	/* pad short strings */
	if (len < 8)
		{
		cp=shortbuf;
		memcpy(shortbuf,buf,len);
		RAND_pseudo_bytes(shortbuf+len, 8-len);
		rnum=8;
		}
	else
		{
		cp=buf;
		rnum=((len+7)/8*8); /* round up to nearest eight */
		}

	if (DES_rw_mode & DES_PCBC_MODE)
		DES_pcbc_encrypt(cp,&(outbuf[HDRSIZE]),(len<8)?8:len,sched,iv,
				 DES_ENCRYPT); 
	else
		DES_cbc_encrypt(cp,&(outbuf[HDRSIZE]),(len<8)?8:len,sched,iv,
				DES_ENCRYPT); 

	/* output */
	outnum=rnum+HDRSIZE;

	for (j=0; j<outnum; j+=i)
		{
		/* eay 26/08/92 I was not doing writing from where we
		 * got up to. */
#ifndef _WIN32
		i=write(fd,(void *)&(outbuf[j]),outnum-j);
#else
		i=_write(fd,(void *)&(outbuf[j]),outnum-j);
#endif
		if (i == -1)
			{
#ifdef EINTR
			if (errno == EINTR)
				i=0;
			else
#endif
			        /* This is really a bad error - very bad
				 * It will stuff-up both ends. */
				return(-1);
			}
		}

	return(len);
#endif /* OPENSSL_NO_POSIX_IO */
	}