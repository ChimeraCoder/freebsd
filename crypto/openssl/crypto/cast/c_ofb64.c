
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

#include <openssl/cast.h>
#include "cast_lcl.h"

/* The input and output encrypted as though 64bit ofb mode is being
 * used.  The extra state information to record how much of the
 * 64bit block we have used is contained in *num;
 */
void CAST_ofb64_encrypt(const unsigned char *in, unsigned char *out,
			long length, const CAST_KEY *schedule, unsigned char *ivec,
			int *num)
	{
	register CAST_LONG v0,v1,t;
	register int n= *num;
	register long l=length;
	unsigned char d[8];
	register char *dp;
	CAST_LONG ti[2];
	unsigned char *iv;
	int save=0;

	iv=ivec;
	n2l(iv,v0);
	n2l(iv,v1);
	ti[0]=v0;
	ti[1]=v1;
	dp=(char *)d;
	l2n(v0,dp);
	l2n(v1,dp);
	while (l--)
		{
		if (n == 0)
			{
			CAST_encrypt((CAST_LONG *)ti,schedule);
			dp=(char *)d;
			t=ti[0]; l2n(t,dp);
			t=ti[1]; l2n(t,dp);
			save++;
			}
		*(out++)= *(in++)^d[n];
		n=(n+1)&0x07;
		}
	if (save)
		{
		v0=ti[0];
		v1=ti[1];
		iv=ivec;
		l2n(v0,iv);
		l2n(v1,iv);
		}
	t=v0=v1=ti[0]=ti[1]=0;
	*num=n;
	}