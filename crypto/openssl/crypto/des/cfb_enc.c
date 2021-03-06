
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

#include "e_os.h"
#include "des_locl.h"
#include <assert.h>

/* The input and output are loaded in multiples of 8 bits.
 * What this means is that if you hame numbits=12 and length=2
 * the first 12 bits will be retrieved from the first byte and half
 * the second.  The second 12 bits will come from the 3rd and half the 4th
 * byte.
 */
/* Until Aug 1 2003 this function did not correctly implement CFB-r, so it
 * will not be compatible with any encryption prior to that date. Ben. */
void DES_cfb_encrypt(const unsigned char *in, unsigned char *out, int numbits,
		     long length, DES_key_schedule *schedule, DES_cblock *ivec,
		     int enc)
	{
	register DES_LONG d0,d1,v0,v1;
	register unsigned long l=length;
	register int num=numbits/8,n=(numbits+7)/8,i,rem=numbits%8;
	DES_LONG ti[2];
	unsigned char *iv;
#ifndef L_ENDIAN
	unsigned char ovec[16];
#else
	unsigned int  sh[4];
	unsigned char *ovec=(unsigned char *)sh;

	/* I kind of count that compiler optimizes away this assertioni,*/
	assert (sizeof(sh[0])==4);	/* as this holds true for all,	*/
					/* but 16-bit platforms...	*/
					
#endif

	if (numbits<=0 || numbits > 64) return;
	iv = &(*ivec)[0];
	c2l(iv,v0);
	c2l(iv,v1);
	if (enc)
		{
		while (l >= (unsigned long)n)
			{
			l-=n;
			ti[0]=v0;
			ti[1]=v1;
			DES_encrypt1((DES_LONG *)ti,schedule,DES_ENCRYPT);
			c2ln(in,d0,d1,n);
			in+=n;
			d0^=ti[0];
			d1^=ti[1];
			l2cn(d0,d1,out,n);
			out+=n;
			/* 30-08-94 - eay - changed because l>>32 and
			 * l<<32 are bad under gcc :-( */
			if (numbits == 32)
				{ v0=v1; v1=d0; }
			else if (numbits == 64)
				{ v0=d0; v1=d1; }
			else
				{
#ifndef L_ENDIAN
				iv=&ovec[0];
				l2c(v0,iv);
				l2c(v1,iv);
				l2c(d0,iv);
				l2c(d1,iv);
#else
				sh[0]=v0, sh[1]=v1, sh[2]=d0, sh[3]=d1;
#endif
				if (rem==0)
					memmove(ovec,ovec+num,8);
				else
					for(i=0 ; i < 8 ; ++i)
						ovec[i]=ovec[i+num]<<rem |
							ovec[i+num+1]>>(8-rem);
#ifdef L_ENDIAN
				v0=sh[0], v1=sh[1];
#else
				iv=&ovec[0];
				c2l(iv,v0);
				c2l(iv,v1);
#endif
				}
			}
		}
	else
		{
		while (l >= (unsigned long)n)
			{
			l-=n;
			ti[0]=v0;
			ti[1]=v1;
			DES_encrypt1((DES_LONG *)ti,schedule,DES_ENCRYPT);
			c2ln(in,d0,d1,n);
			in+=n;
			/* 30-08-94 - eay - changed because l>>32 and
			 * l<<32 are bad under gcc :-( */
			if (numbits == 32)
				{ v0=v1; v1=d0; }
			else if (numbits == 64)
				{ v0=d0; v1=d1; }
			else
				{
#ifndef L_ENDIAN
				iv=&ovec[0];
				l2c(v0,iv);
				l2c(v1,iv);
				l2c(d0,iv);
				l2c(d1,iv);
#else
				sh[0]=v0, sh[1]=v1, sh[2]=d0, sh[3]=d1;
#endif
				if (rem==0)
					memmove(ovec,ovec+num,8);
				else
					for(i=0 ; i < 8 ; ++i)
						ovec[i]=ovec[i+num]<<rem |
							ovec[i+num+1]>>(8-rem);
#ifdef L_ENDIAN
				v0=sh[0], v1=sh[1];
#else
				iv=&ovec[0];
				c2l(iv,v0);
				c2l(iv,v1);
#endif
				}
			d0^=ti[0];
			d1^=ti[1];
			l2cn(d0,d1,out,n);
			out+=n;
			}
		}
	iv = &(*ivec)[0];
	l2c(v0,iv);
	l2c(v1,iv);
	v0=v1=d0=d1=ti[0]=ti[1]=0;
	}