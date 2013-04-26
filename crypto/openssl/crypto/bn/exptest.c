
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
#include <stdlib.h>
#include <string.h>

#include "../e_os.h"

#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#define NUM_BITS	(BN_BITS*2)

static const char rnd_seed[] = "string to make the random number generator think it has entropy";

int main(int argc, char *argv[])
	{
	BN_CTX *ctx;
	BIO *out=NULL;
	int i,ret;
	unsigned char c;
	BIGNUM *r_mont,*r_mont_const,*r_recp,*r_simple,*a,*b,*m;

	RAND_seed(rnd_seed, sizeof rnd_seed); /* or BN_rand may fail, and we don't
	                                       * even check its return value
	                                       * (which we should) */

	ERR_load_BN_strings();

	ctx=BN_CTX_new();
	if (ctx == NULL) EXIT(1);
	r_mont=BN_new();
	r_mont_const=BN_new();
	r_recp=BN_new();
	r_simple=BN_new();
	a=BN_new();
	b=BN_new();
	m=BN_new();
	if (	(r_mont == NULL) || (r_recp == NULL) ||
		(a == NULL) || (b == NULL))
		goto err;

	out=BIO_new(BIO_s_file());

	if (out == NULL) EXIT(1);
	BIO_set_fp(out,stdout,BIO_NOCLOSE);

	for (i=0; i<200; i++)
		{
		RAND_bytes(&c,1);
		c=(c%BN_BITS)-BN_BITS2;
		BN_rand(a,NUM_BITS+c,0,0);

		RAND_bytes(&c,1);
		c=(c%BN_BITS)-BN_BITS2;
		BN_rand(b,NUM_BITS+c,0,0);

		RAND_bytes(&c,1);
		c=(c%BN_BITS)-BN_BITS2;
		BN_rand(m,NUM_BITS+c,0,1);

		BN_mod(a,a,m,ctx);
		BN_mod(b,b,m,ctx);

		ret=BN_mod_exp_mont(r_mont,a,b,m,ctx,NULL);
		if (ret <= 0)
			{
			printf("BN_mod_exp_mont() problems\n");
			ERR_print_errors(out);
			EXIT(1);
			}

		ret=BN_mod_exp_recp(r_recp,a,b,m,ctx);
		if (ret <= 0)
			{
			printf("BN_mod_exp_recp() problems\n");
			ERR_print_errors(out);
			EXIT(1);
			}

		ret=BN_mod_exp_simple(r_simple,a,b,m,ctx);
		if (ret <= 0)
			{
			printf("BN_mod_exp_simple() problems\n");
			ERR_print_errors(out);
			EXIT(1);
			}

		ret=BN_mod_exp_mont_consttime(r_mont_const,a,b,m,ctx,NULL);
		if (ret <= 0)
			{
			printf("BN_mod_exp_mont_consttime() problems\n");
			ERR_print_errors(out);
			EXIT(1);
			}

		if (BN_cmp(r_simple, r_mont) == 0
		    && BN_cmp(r_simple,r_recp) == 0
			&& BN_cmp(r_simple,r_mont_const) == 0)
			{
			printf(".");
			fflush(stdout);
			}
		else
		  	{
			if (BN_cmp(r_simple,r_mont) != 0)
				printf("\nsimple and mont results differ\n");
			if (BN_cmp(r_simple,r_mont_const) != 0)
				printf("\nsimple and mont const time results differ\n");
			if (BN_cmp(r_simple,r_recp) != 0)
				printf("\nsimple and recp results differ\n");

			printf("a (%3d) = ",BN_num_bits(a));   BN_print(out,a);
			printf("\nb (%3d) = ",BN_num_bits(b)); BN_print(out,b);
			printf("\nm (%3d) = ",BN_num_bits(m)); BN_print(out,m);
			printf("\nsimple   =");	BN_print(out,r_simple);
			printf("\nrecp     =");	BN_print(out,r_recp);
			printf("\nmont     ="); BN_print(out,r_mont);
			printf("\nmont_ct  ="); BN_print(out,r_mont_const);
			printf("\n");
			EXIT(1);
			}
		}
	BN_free(r_mont);
	BN_free(r_mont_const);
	BN_free(r_recp);
	BN_free(r_simple);
	BN_free(a);
	BN_free(b);
	BN_free(m);
	BN_CTX_free(ctx);
	ERR_remove_thread_state(NULL);
	CRYPTO_mem_leaks(out);
	BIO_free(out);
	printf(" done\n");
	EXIT(0);
err:
	ERR_load_crypto_strings();
	ERR_print_errors(out);
#ifdef OPENSSL_SYS_NETWARE
    printf("ERROR\n");
#endif
	EXIT(1);
	return(1);
	}