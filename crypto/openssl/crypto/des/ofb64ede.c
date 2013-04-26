
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

#include "des_locl.h"

/* The input and output encrypted as though 64bit ofb mode is being
 * used.  The extra state information to record how much of the
 * 64bit block we have used is contained in *num;
 */
void DES_ede3_ofb64_encrypt(register const unsigned char *in,
			    register unsigned char *out, long length,
			    DES_key_schedule *k1, DES_key_schedule *k2,
			    DES_key_schedule *k3, DES_cblock *ivec,
			    int *num)
	{
	register DES_LONG v0,v1;
	register int n= *num;
	register long l=length;
	DES_cblock d;
	register char *dp;
	DES_LONG ti[2];
	unsigned char *iv;
	int save=0;

	iv = &(*ivec)[0];
	c2l(iv,v0);
	c2l(iv,v1);
	ti[0]=v0;
	ti[1]=v1;
	dp=(char *)d;
	l2c(v0,dp);
	l2c(v1,dp);
	while (l--)
		{
		if (n == 0)
			{
			/* ti[0]=v0; */
			/* ti[1]=v1; */
			DES_encrypt3(ti,k1,k2,k3);
			v0=ti[0];
			v1=ti[1];

			dp=(char *)d;
			l2c(v0,dp);
			l2c(v1,dp);
			save++;
			}
		*(out++)= *(in++)^d[n];
		n=(n+1)&0x07;
		}
	if (save)
		{
/*		v0=ti[0];
		v1=ti[1];*/
		iv = &(*ivec)[0];
		l2c(v0,iv);
		l2c(v1,iv);
		}
	v0=v1=ti[0]=ti[1]=0;
	*num=n;
	}

#ifdef undef /* MACRO */
void DES_ede2_ofb64_encrypt(register unsigned char *in,
	     register unsigned char *out, long length, DES_key_schedule k1,
	     DES_key_schedule k2, DES_cblock (*ivec), int *num)
	{
	DES_ede3_ofb64_encrypt(in, out, length, k1,k2,k1, ivec, num);
	}
#endif