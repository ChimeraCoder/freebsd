
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
#include <time.h>
#include "cryptlib.h"
#include "bn_lcl.h"
#include <openssl/rand.h>

static int bnrand(int pseudorand, BIGNUM *rnd, int bits, int top, int bottom)
	{
	unsigned char *buf=NULL;
	int ret=0,bit,bytes,mask;
	time_t tim;

	if (bits == 0)
		{
		BN_zero(rnd);
		return 1;
		}

	bytes=(bits+7)/8;
	bit=(bits-1)%8;
	mask=0xff<<(bit+1);

	buf=(unsigned char *)OPENSSL_malloc(bytes);
	if (buf == NULL)
		{
		BNerr(BN_F_BNRAND,ERR_R_MALLOC_FAILURE);
		goto err;
		}

	/* make a random number and set the top and bottom bits */
	time(&tim);
	RAND_add(&tim,sizeof(tim),0.0);

	if (pseudorand)
		{
		if (RAND_pseudo_bytes(buf, bytes) == -1)
			goto err;
		}
	else
		{
		if (RAND_bytes(buf, bytes) <= 0)
			goto err;
		}

#if 1
	if (pseudorand == 2)
		{
		/* generate patterns that are more likely to trigger BN
		   library bugs */
		int i;
		unsigned char c;

		for (i = 0; i < bytes; i++)
			{
			RAND_pseudo_bytes(&c, 1);
			if (c >= 128 && i > 0)
				buf[i] = buf[i-1];
			else if (c < 42)
				buf[i] = 0;
			else if (c < 84)
				buf[i] = 255;
			}
		}
#endif

	if (top != -1)
		{
		if (top)
			{
			if (bit == 0)
				{
				buf[0]=1;
				buf[1]|=0x80;
				}
			else
				{
				buf[0]|=(3<<(bit-1));
				}
			}
		else
			{
			buf[0]|=(1<<bit);
			}
		}
	buf[0] &= ~mask;
	if (bottom) /* set bottom bit if requested */
		buf[bytes-1]|=1;
	if (!BN_bin2bn(buf,bytes,rnd)) goto err;
	ret=1;
err:
	if (buf != NULL)
		{
		OPENSSL_cleanse(buf,bytes);
		OPENSSL_free(buf);
		}
	bn_check_top(rnd);
	return(ret);
	}

int     BN_rand(BIGNUM *rnd, int bits, int top, int bottom)
	{
	return bnrand(0, rnd, bits, top, bottom);
	}

int     BN_pseudo_rand(BIGNUM *rnd, int bits, int top, int bottom)
	{
	return bnrand(1, rnd, bits, top, bottom);
	}

#if 1
int     BN_bntest_rand(BIGNUM *rnd, int bits, int top, int bottom)
	{
	return bnrand(2, rnd, bits, top, bottom);
	}
#endif


/* random number r:  0 <= r < range */
static int bn_rand_range(int pseudo, BIGNUM *r, const BIGNUM *range)
	{
	int (*bn_rand)(BIGNUM *, int, int, int) = pseudo ? BN_pseudo_rand : BN_rand;
	int n;
	int count = 100;

	if (range->neg || BN_is_zero(range))
		{
		BNerr(BN_F_BN_RAND_RANGE, BN_R_INVALID_RANGE);
		return 0;
		}

	n = BN_num_bits(range); /* n > 0 */

	/* BN_is_bit_set(range, n - 1) always holds */

	if (n == 1)
		BN_zero(r);
	else if (!BN_is_bit_set(range, n - 2) && !BN_is_bit_set(range, n - 3))
		{
		/* range = 100..._2,
		 * so  3*range (= 11..._2)  is exactly one bit longer than  range */
		do
			{
			if (!bn_rand(r, n + 1, -1, 0)) return 0;
			/* If  r < 3*range,  use  r := r MOD range
			 * (which is either  r, r - range,  or  r - 2*range).
			 * Otherwise, iterate once more.
			 * Since  3*range = 11..._2, each iteration succeeds with
			 * probability >= .75. */
			if (BN_cmp(r ,range) >= 0)
				{
				if (!BN_sub(r, r, range)) return 0;
				if (BN_cmp(r, range) >= 0)
					if (!BN_sub(r, r, range)) return 0;
				}

			if (!--count)
				{
				BNerr(BN_F_BN_RAND_RANGE, BN_R_TOO_MANY_ITERATIONS);
				return 0;
				}
			
			}
		while (BN_cmp(r, range) >= 0);
		}
	else
		{
		do
			{
			/* range = 11..._2  or  range = 101..._2 */
			if (!bn_rand(r, n, -1, 0)) return 0;

			if (!--count)
				{
				BNerr(BN_F_BN_RAND_RANGE, BN_R_TOO_MANY_ITERATIONS);
				return 0;
				}
			}
		while (BN_cmp(r, range) >= 0);
		}

	bn_check_top(r);
	return 1;
	}


int	BN_rand_range(BIGNUM *r, const BIGNUM *range)
	{
	return bn_rand_range(0, r, range);
	}

int	BN_pseudo_rand_range(BIGNUM *r, const BIGNUM *range)
	{
	return bn_rand_range(1, r, range);
	}