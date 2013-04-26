
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
#include <openssl/rand.h>

#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif

#ifdef OPENSSL_FIPS
#include <openssl/fips.h>
#include <openssl/fips_rand.h>
#endif

#ifndef OPENSSL_NO_ENGINE
/* non-NULL if default_RAND_meth is ENGINE-provided */
static ENGINE *funct_ref =NULL;
#endif
static const RAND_METHOD *default_RAND_meth = NULL;

int RAND_set_rand_method(const RAND_METHOD *meth)
	{
#ifndef OPENSSL_NO_ENGINE
	if(funct_ref)
		{
		ENGINE_finish(funct_ref);
		funct_ref = NULL;
		}
#endif
	default_RAND_meth = meth;
	return 1;
	}

const RAND_METHOD *RAND_get_rand_method(void)
	{
	if (!default_RAND_meth)
		{
#ifndef OPENSSL_NO_ENGINE
		ENGINE *e = ENGINE_get_default_RAND();
		if(e)
			{
			default_RAND_meth = ENGINE_get_RAND(e);
			if(!default_RAND_meth)
				{
				ENGINE_finish(e);
				e = NULL;
				}
			}
		if(e)
			funct_ref = e;
		else
#endif
			default_RAND_meth = RAND_SSLeay();
		}
	return default_RAND_meth;
	}

#ifndef OPENSSL_NO_ENGINE
int RAND_set_rand_engine(ENGINE *engine)
	{
	const RAND_METHOD *tmp_meth = NULL;
	if(engine)
		{
		if(!ENGINE_init(engine))
			return 0;
		tmp_meth = ENGINE_get_RAND(engine);
		if(!tmp_meth)
			{
			ENGINE_finish(engine);
			return 0;
			}
		}
	/* This function releases any prior ENGINE so call it first */
	RAND_set_rand_method(tmp_meth);
	funct_ref = engine;
	return 1;
	}
#endif

void RAND_cleanup(void)
	{
	const RAND_METHOD *meth = RAND_get_rand_method();
	if (meth && meth->cleanup)
		meth->cleanup();
	RAND_set_rand_method(NULL);
	}

void RAND_seed(const void *buf, int num)
	{
	const RAND_METHOD *meth = RAND_get_rand_method();
	if (meth && meth->seed)
		meth->seed(buf,num);
	}

void RAND_add(const void *buf, int num, double entropy)
	{
	const RAND_METHOD *meth = RAND_get_rand_method();
	if (meth && meth->add)
		meth->add(buf,num,entropy);
	}

int RAND_bytes(unsigned char *buf, int num)
	{
	const RAND_METHOD *meth = RAND_get_rand_method();
	if (meth && meth->bytes)
		return meth->bytes(buf,num);
	return(-1);
	}

int RAND_pseudo_bytes(unsigned char *buf, int num)
	{
	const RAND_METHOD *meth = RAND_get_rand_method();
	if (meth && meth->pseudorand)
		return meth->pseudorand(buf,num);
	return(-1);
	}

int RAND_status(void)
	{
	const RAND_METHOD *meth = RAND_get_rand_method();
	if (meth && meth->status)
		return meth->status();
	return 0;
	}

#ifdef OPENSSL_FIPS

/* FIPS DRBG initialisation code. This sets up the DRBG for use by the
 * rest of OpenSSL. 
 */

/* Entropy gatherer: use standard OpenSSL PRNG to seed (this will gather
 * entropy internally through RAND_poll().
 */

static size_t drbg_get_entropy(DRBG_CTX *ctx, unsigned char **pout,
                                int entropy, size_t min_len, size_t max_len)
        {
	/* Round up request to multiple of block size */
	min_len = ((min_len + 19) / 20) * 20;
	*pout = OPENSSL_malloc(min_len);
	if (!*pout)
		return 0;
	if (RAND_SSLeay()->bytes(*pout, min_len) <= 0)
		{
		OPENSSL_free(*pout);
		*pout = NULL;
		return 0;
		}
        return min_len;
        }

static void drbg_free_entropy(DRBG_CTX *ctx, unsigned char *out, size_t olen)
	{
	if (out)
		{
		OPENSSL_cleanse(out, olen);
		OPENSSL_free(out);
		}
	}

/* Set "additional input" when generating random data. This uses the
 * current PID, a time value and a counter.
 */

static size_t drbg_get_adin(DRBG_CTX *ctx, unsigned char **pout)
    	{
	/* Use of static variables is OK as this happens under a lock */
	static unsigned char buf[16];
	static unsigned long counter;
	FIPS_get_timevec(buf, &counter);
	*pout = buf;
	return sizeof(buf);
	}

/* RAND_add() and RAND_seed() pass through to OpenSSL PRNG so it is 
 * correctly seeded by RAND_poll().
 */

static int drbg_rand_add(DRBG_CTX *ctx, const void *in, int inlen,
				double entropy)
	{
	RAND_SSLeay()->add(in, inlen, entropy);
	return 1;
	}

static int drbg_rand_seed(DRBG_CTX *ctx, const void *in, int inlen)
	{
	RAND_SSLeay()->seed(in, inlen);
	return 1;
	}

#ifndef OPENSSL_DRBG_DEFAULT_TYPE
#define OPENSSL_DRBG_DEFAULT_TYPE	NID_aes_256_ctr
#endif
#ifndef OPENSSL_DRBG_DEFAULT_FLAGS
#define OPENSSL_DRBG_DEFAULT_FLAGS	DRBG_FLAG_CTR_USE_DF
#endif 

static int fips_drbg_type = OPENSSL_DRBG_DEFAULT_TYPE;
static int fips_drbg_flags = OPENSSL_DRBG_DEFAULT_FLAGS;

void RAND_set_fips_drbg_type(int type, int flags)
	{
	fips_drbg_type = type;
	fips_drbg_flags = flags;
	}

int RAND_init_fips(void)
	{
	DRBG_CTX *dctx;
	size_t plen;
	unsigned char pers[32], *p;
	dctx = FIPS_get_default_drbg();
        if (FIPS_drbg_init(dctx, fips_drbg_type, fips_drbg_flags) <= 0)
		{
		RANDerr(RAND_F_RAND_INIT_FIPS, RAND_R_ERROR_INITIALISING_DRBG);
		return 0;
		}
		
        FIPS_drbg_set_callbacks(dctx,
				drbg_get_entropy, drbg_free_entropy, 20,
				drbg_get_entropy, drbg_free_entropy);
	FIPS_drbg_set_rand_callbacks(dctx, drbg_get_adin, 0,
					drbg_rand_seed, drbg_rand_add);
	/* Personalisation string: a string followed by date time vector */
	strcpy((char *)pers, "OpenSSL DRBG2.0");
	plen = drbg_get_adin(dctx, &p);
	memcpy(pers + 16, p, plen);

        if (FIPS_drbg_instantiate(dctx, pers, sizeof(pers)) <= 0)
		{
		RANDerr(RAND_F_RAND_INIT_FIPS, RAND_R_ERROR_INSTANTIATING_DRBG);
		return 0;
		}
        FIPS_rand_set_method(FIPS_drbg_method());
	return 1;
	}

#endif