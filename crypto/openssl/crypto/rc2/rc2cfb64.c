
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

#include <openssl/rc2.h>
#include "rc2_locl.h"

/* The input and output encrypted as though 64bit cfb mode is being
 * used.  The extra state information to record how much of the
 * 64bit block we have used is contained in *num;
 */

void RC2_cfb64_encrypt(const unsigned char *in, unsigned char *out,
		       long length, RC2_KEY *schedule, unsigned char *ivec,
		       int *num, int encrypt)
	{
	register unsigned long v0,v1,t;
	register int n= *num;
	register long l=length;
	unsigned long ti[2];
	unsigned char *iv,c,cc;

	iv=(unsigned char *)ivec;
	if (encrypt)
		{
		while (l--)
			{
			if (n == 0)
				{
				c2l(iv,v0); ti[0]=v0;
				c2l(iv,v1); ti[1]=v1;
				RC2_encrypt((unsigned long *)ti,schedule);
				iv=(unsigned char *)ivec;
				t=ti[0]; l2c(t,iv);
				t=ti[1]; l2c(t,iv);
				iv=(unsigned char *)ivec;
				}
			c= *(in++)^iv[n];
			*(out++)=c;
			iv[n]=c;
			n=(n+1)&0x07;
			}
		}
	else
		{
		while (l--)
			{
			if (n == 0)
				{
				c2l(iv,v0); ti[0]=v0;
				c2l(iv,v1); ti[1]=v1;
				RC2_encrypt((unsigned long *)ti,schedule);
				iv=(unsigned char *)ivec;
				t=ti[0]; l2c(t,iv);
				t=ti[1]; l2c(t,iv);
				iv=(unsigned char *)ivec;
				}
			cc= *(in++);
			c=iv[n];
			iv[n]=cc;
			*(out++)=c^cc;
			n=(n+1)&0x07;
			}
		}
	v0=v1=ti[0]=ti[1]=t=c=cc=0;
	*num=n;
	}