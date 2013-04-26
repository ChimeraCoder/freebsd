
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
#include "cryptlib.h"
#include "bn_lcl.h"

int BN_bn2mpi(const BIGNUM *a, unsigned char *d)
	{
	int bits;
	int num=0;
	int ext=0;
	long l;

	bits=BN_num_bits(a);
	num=(bits+7)/8;
	if (bits > 0)
		{
		ext=((bits & 0x07) == 0);
		}
	if (d == NULL)
		return(num+4+ext);

	l=num+ext;
	d[0]=(unsigned char)(l>>24)&0xff;
	d[1]=(unsigned char)(l>>16)&0xff;
	d[2]=(unsigned char)(l>> 8)&0xff;
	d[3]=(unsigned char)(l    )&0xff;
	if (ext) d[4]=0;
	num=BN_bn2bin(a,&(d[4+ext]));
	if (a->neg)
		d[4]|=0x80;
	return(num+4+ext);
	}

BIGNUM *BN_mpi2bn(const unsigned char *d, int n, BIGNUM *a)
	{
	long len;
	int neg=0;

	if (n < 4)
		{
		BNerr(BN_F_BN_MPI2BN,BN_R_INVALID_LENGTH);
		return(NULL);
		}
	len=((long)d[0]<<24)|((long)d[1]<<16)|((int)d[2]<<8)|(int)d[3];
	if ((len+4) != n)
		{
		BNerr(BN_F_BN_MPI2BN,BN_R_ENCODING_ERROR);
		return(NULL);
		}

	if (a == NULL) a=BN_new();
	if (a == NULL) return(NULL);

	if (len == 0)
		{
		a->neg=0;
		a->top=0;
		return(a);
		}
	d+=4;
	if ((*d) & 0x80)
		neg=1;
	if (BN_bin2bn(d,(int)len,a) == NULL)
		return(NULL);
	a->neg=neg;
	if (neg)
		{
		BN_clear_bit(a,BN_num_bits(a)-1);
		}
	bn_check_top(a);
	return(a);
	}