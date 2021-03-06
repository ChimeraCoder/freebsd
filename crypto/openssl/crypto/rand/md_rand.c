
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

#define OPENSSL_FIPSEVP

#ifdef MD_RAND_DEBUG
# ifndef NDEBUG
#   define NDEBUG
# endif
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "e_os.h"

#include <openssl/crypto.h>
#include <openssl/rand.h>
#include "rand_lcl.h"

#include <openssl/err.h>

#ifdef BN_DEBUG
# define PREDICT
#endif

/* #define PREDICT	1 */

#define STATE_SIZE	1023
static int state_num=0,state_index=0;
static unsigned char state[STATE_SIZE+MD_DIGEST_LENGTH];
static unsigned char md[MD_DIGEST_LENGTH];
static long md_count[2]={0,0};
static double entropy=0;
static int initialized=0;

static unsigned int crypto_lock_rand = 0; /* may be set only when a thread
                                           * holds CRYPTO_LOCK_RAND
                                           * (to prevent double locking) */
/* access to lockin_thread is synchronized by CRYPTO_LOCK_RAND2 */
static CRYPTO_THREADID locking_threadid; /* valid iff crypto_lock_rand is set */


#ifdef PREDICT
int rand_predictable=0;
#endif

const char RAND_version[]="RAND" OPENSSL_VERSION_PTEXT;

static void ssleay_rand_cleanup(void);
static void ssleay_rand_seed(const void *buf, int num);
static void ssleay_rand_add(const void *buf, int num, double add_entropy);
static int ssleay_rand_bytes(unsigned char *buf, int num, int pseudo);
static int ssleay_rand_nopseudo_bytes(unsigned char *buf, int num);
static int ssleay_rand_pseudo_bytes(unsigned char *buf, int num);
static int ssleay_rand_status(void);

RAND_METHOD rand_ssleay_meth={
	ssleay_rand_seed,
	ssleay_rand_nopseudo_bytes,
	ssleay_rand_cleanup,
	ssleay_rand_add,
	ssleay_rand_pseudo_bytes,
	ssleay_rand_status
	}; 

RAND_METHOD *RAND_SSLeay(void)
	{
	return(&rand_ssleay_meth);
	}

static void ssleay_rand_cleanup(void)
	{
	OPENSSL_cleanse(state,sizeof(state));
	state_num=0;
	state_index=0;
	OPENSSL_cleanse(md,MD_DIGEST_LENGTH);
	md_count[0]=0;
	md_count[1]=0;
	entropy=0;
	initialized=0;
	}

static void ssleay_rand_add(const void *buf, int num, double add)
	{
	int i,j,k,st_idx;
	long md_c[2];
	unsigned char local_md[MD_DIGEST_LENGTH];
	EVP_MD_CTX m;
	int do_not_lock;

	/*
	 * (Based on the rand(3) manpage)
	 *
	 * The input is chopped up into units of 20 bytes (or less for
	 * the last block).  Each of these blocks is run through the hash
	 * function as follows:  The data passed to the hash function
	 * is the current 'md', the same number of bytes from the 'state'
	 * (the location determined by in incremented looping index) as
	 * the current 'block', the new key data 'block', and 'count'
	 * (which is incremented after each use).
	 * The result of this is kept in 'md' and also xored into the
	 * 'state' at the same locations that were used as input into the
         * hash function.
	 */

	/* check if we already have the lock */
	if (crypto_lock_rand)
		{
		CRYPTO_THREADID cur;
		CRYPTO_THREADID_current(&cur);
		CRYPTO_r_lock(CRYPTO_LOCK_RAND2);
		do_not_lock = !CRYPTO_THREADID_cmp(&locking_threadid, &cur);
		CRYPTO_r_unlock(CRYPTO_LOCK_RAND2);
		}
	else
		do_not_lock = 0;

	if (!do_not_lock) CRYPTO_w_lock(CRYPTO_LOCK_RAND);
	st_idx=state_index;

	/* use our own copies of the counters so that even
	 * if a concurrent thread seeds with exactly the
	 * same data and uses the same subarray there's _some_
	 * difference */
	md_c[0] = md_count[0];
	md_c[1] = md_count[1];

	memcpy(local_md, md, sizeof md);

	/* state_index <= state_num <= STATE_SIZE */
	state_index += num;
	if (state_index >= STATE_SIZE)
		{
		state_index%=STATE_SIZE;
		state_num=STATE_SIZE;
		}
	else if (state_num < STATE_SIZE)	
		{
		if (state_index > state_num)
			state_num=state_index;
		}
	/* state_index <= state_num <= STATE_SIZE */

	/* state[st_idx], ..., state[(st_idx + num - 1) % STATE_SIZE]
	 * are what we will use now, but other threads may use them
	 * as well */

	md_count[1] += (num / MD_DIGEST_LENGTH) + (num % MD_DIGEST_LENGTH > 0);

	if (!do_not_lock) CRYPTO_w_unlock(CRYPTO_LOCK_RAND);

	EVP_MD_CTX_init(&m);
	for (i=0; i<num; i+=MD_DIGEST_LENGTH)
		{
		j=(num-i);
		j=(j > MD_DIGEST_LENGTH)?MD_DIGEST_LENGTH:j;

		MD_Init(&m);
		MD_Update(&m,local_md,MD_DIGEST_LENGTH);
		k=(st_idx+j)-STATE_SIZE;
		if (k > 0)
			{
			MD_Update(&m,&(state[st_idx]),j-k);
			MD_Update(&m,&(state[0]),k);
			}
		else
			MD_Update(&m,&(state[st_idx]),j);

		/* DO NOT REMOVE THE FOLLOWING CALL TO MD_Update()! */
		MD_Update(&m,buf,j);
		/* We know that line may cause programs such as
		   purify and valgrind to complain about use of
		   uninitialized data.  The problem is not, it's
		   with the caller.  Removing that line will make
		   sure you get really bad randomness and thereby
		   other problems such as very insecure keys. */

		MD_Update(&m,(unsigned char *)&(md_c[0]),sizeof(md_c));
		MD_Final(&m,local_md);
		md_c[1]++;

		buf=(const char *)buf + j;

		for (k=0; k<j; k++)
			{
			/* Parallel threads may interfere with this,
			 * but always each byte of the new state is
			 * the XOR of some previous value of its
			 * and local_md (itermediate values may be lost).
			 * Alway using locking could hurt performance more
			 * than necessary given that conflicts occur only
			 * when the total seeding is longer than the random
			 * state. */
			state[st_idx++]^=local_md[k];
			if (st_idx >= STATE_SIZE)
				st_idx=0;
			}
		}
	EVP_MD_CTX_cleanup(&m);

	if (!do_not_lock) CRYPTO_w_lock(CRYPTO_LOCK_RAND);
	/* Don't just copy back local_md into md -- this could mean that
	 * other thread's seeding remains without effect (except for
	 * the incremented counter).  By XORing it we keep at least as
	 * much entropy as fits into md. */
	for (k = 0; k < (int)sizeof(md); k++)
		{
		md[k] ^= local_md[k];
		}
	if (entropy < ENTROPY_NEEDED) /* stop counting when we have enough */
	    entropy += add;
	if (!do_not_lock) CRYPTO_w_unlock(CRYPTO_LOCK_RAND);
	
#if !defined(OPENSSL_THREADS) && !defined(OPENSSL_SYS_WIN32)
	assert(md_c[1] == md_count[1]);
#endif
	}

static void ssleay_rand_seed(const void *buf, int num)
	{
	ssleay_rand_add(buf, num, (double)num);
	}

static int ssleay_rand_bytes(unsigned char *buf, int num, int pseudo)
	{
	static volatile int stirred_pool = 0;
	int i,j,k,st_num,st_idx;
	int num_ceil;
	int ok;
	long md_c[2];
	unsigned char local_md[MD_DIGEST_LENGTH];
	EVP_MD_CTX m;
#ifndef GETPID_IS_MEANINGLESS
	pid_t curr_pid = getpid();
#endif
	int do_stir_pool = 0;

#ifdef PREDICT
	if (rand_predictable)
		{
		static unsigned char val=0;

		for (i=0; i<num; i++)
			buf[i]=val++;
		return(1);
		}
#endif

	if (num <= 0)
		return 1;

	EVP_MD_CTX_init(&m);
	/* round upwards to multiple of MD_DIGEST_LENGTH/2 */
	num_ceil = (1 + (num-1)/(MD_DIGEST_LENGTH/2)) * (MD_DIGEST_LENGTH/2);

	/*
	 * (Based on the rand(3) manpage:)
	 *
	 * For each group of 10 bytes (or less), we do the following:
	 *
	 * Input into the hash function the local 'md' (which is initialized from
	 * the global 'md' before any bytes are generated), the bytes that are to
	 * be overwritten by the random bytes, and bytes from the 'state'
	 * (incrementing looping index). From this digest output (which is kept
	 * in 'md'), the top (up to) 10 bytes are returned to the caller and the
	 * bottom 10 bytes are xored into the 'state'.
	 * 
	 * Finally, after we have finished 'num' random bytes for the
	 * caller, 'count' (which is incremented) and the local and global 'md'
	 * are fed into the hash function and the results are kept in the
	 * global 'md'.
	 */

	CRYPTO_w_lock(CRYPTO_LOCK_RAND);

	/* prevent ssleay_rand_bytes() from trying to obtain the lock again */
	CRYPTO_w_lock(CRYPTO_LOCK_RAND2);
	CRYPTO_THREADID_current(&locking_threadid);
	CRYPTO_w_unlock(CRYPTO_LOCK_RAND2);
	crypto_lock_rand = 1;

	if (!initialized)
		{
		RAND_poll();
		initialized = 1;
		}
	
	if (!stirred_pool)
		do_stir_pool = 1;
	
	ok = (entropy >= ENTROPY_NEEDED);
	if (!ok)
		{
		/* If the PRNG state is not yet unpredictable, then seeing
		 * the PRNG output may help attackers to determine the new
		 * state; thus we have to decrease the entropy estimate.
		 * Once we've had enough initial seeding we don't bother to
		 * adjust the entropy count, though, because we're not ambitious
		 * to provide *information-theoretic* randomness.
		 *
		 * NOTE: This approach fails if the program forks before
		 * we have enough entropy. Entropy should be collected
		 * in a separate input pool and be transferred to the
		 * output pool only when the entropy limit has been reached.
		 */
		entropy -= num;
		if (entropy < 0)
			entropy = 0;
		}

	if (do_stir_pool)
		{
		/* In the output function only half of 'md' remains secret,
		 * so we better make sure that the required entropy gets
		 * 'evenly distributed' through 'state', our randomness pool.
		 * The input function (ssleay_rand_add) chains all of 'md',
		 * which makes it more suitable for this purpose.
		 */

		int n = STATE_SIZE; /* so that the complete pool gets accessed */
		while (n > 0)
			{
#if MD_DIGEST_LENGTH > 20
# error "Please adjust DUMMY_SEED."
#endif
#define DUMMY_SEED "...................." /* at least MD_DIGEST_LENGTH */
			/* Note that the seed does not matter, it's just that
			 * ssleay_rand_add expects to have something to hash. */
			ssleay_rand_add(DUMMY_SEED, MD_DIGEST_LENGTH, 0.0);
			n -= MD_DIGEST_LENGTH;
			}
		if (ok)
			stirred_pool = 1;
		}

	st_idx=state_index;
	st_num=state_num;
	md_c[0] = md_count[0];
	md_c[1] = md_count[1];
	memcpy(local_md, md, sizeof md);

	state_index+=num_ceil;
	if (state_index > state_num)
		state_index %= state_num;

	/* state[st_idx], ..., state[(st_idx + num_ceil - 1) % st_num]
	 * are now ours (but other threads may use them too) */

	md_count[0] += 1;

	/* before unlocking, we must clear 'crypto_lock_rand' */
	crypto_lock_rand = 0;
	CRYPTO_w_unlock(CRYPTO_LOCK_RAND);

	while (num > 0)
		{
		/* num_ceil -= MD_DIGEST_LENGTH/2 */
		j=(num >= MD_DIGEST_LENGTH/2)?MD_DIGEST_LENGTH/2:num;
		num-=j;
		MD_Init(&m);
#ifndef GETPID_IS_MEANINGLESS
		if (curr_pid) /* just in the first iteration to save time */
			{
			MD_Update(&m,(unsigned char*)&curr_pid,sizeof curr_pid);
			curr_pid = 0;
			}
#endif
		MD_Update(&m,local_md,MD_DIGEST_LENGTH);
		MD_Update(&m,(unsigned char *)&(md_c[0]),sizeof(md_c));

#ifndef PURIFY /* purify complains */
		/* The following line uses the supplied buffer as a small
		 * source of entropy: since this buffer is often uninitialised
		 * it may cause programs such as purify or valgrind to
		 * complain. So for those builds it is not used: the removal
		 * of such a small source of entropy has negligible impact on
		 * security.
		 */
		MD_Update(&m,buf,j);
#endif

		k=(st_idx+MD_DIGEST_LENGTH/2)-st_num;
		if (k > 0)
			{
			MD_Update(&m,&(state[st_idx]),MD_DIGEST_LENGTH/2-k);
			MD_Update(&m,&(state[0]),k);
			}
		else
			MD_Update(&m,&(state[st_idx]),MD_DIGEST_LENGTH/2);
		MD_Final(&m,local_md);

		for (i=0; i<MD_DIGEST_LENGTH/2; i++)
			{
			state[st_idx++]^=local_md[i]; /* may compete with other threads */
			if (st_idx >= st_num)
				st_idx=0;
			if (i < j)
				*(buf++)=local_md[i+MD_DIGEST_LENGTH/2];
			}
		}

	MD_Init(&m);
	MD_Update(&m,(unsigned char *)&(md_c[0]),sizeof(md_c));
	MD_Update(&m,local_md,MD_DIGEST_LENGTH);
	CRYPTO_w_lock(CRYPTO_LOCK_RAND);
	MD_Update(&m,md,MD_DIGEST_LENGTH);
	MD_Final(&m,md);
	CRYPTO_w_unlock(CRYPTO_LOCK_RAND);

	EVP_MD_CTX_cleanup(&m);
	if (ok)
		return(1);
	else if (pseudo)
		return 0;
	else 
		{
		RANDerr(RAND_F_SSLEAY_RAND_BYTES,RAND_R_PRNG_NOT_SEEDED);
		ERR_add_error_data(1, "You need to read the OpenSSL FAQ, "
			"http://www.openssl.org/support/faq.html");
		return(0);
		}
	}

static int ssleay_rand_nopseudo_bytes(unsigned char *buf, int num)
	{
	return ssleay_rand_bytes(buf, num, 0);
	}

/* pseudo-random bytes that are guaranteed to be unique but not
   unpredictable */
static int ssleay_rand_pseudo_bytes(unsigned char *buf, int num) 
	{
	return ssleay_rand_bytes(buf, num, 1);
	}

static int ssleay_rand_status(void)
	{
	CRYPTO_THREADID cur;
	int ret;
	int do_not_lock;

	CRYPTO_THREADID_current(&cur);
	/* check if we already have the lock
	 * (could happen if a RAND_poll() implementation calls RAND_status()) */
	if (crypto_lock_rand)
		{
		CRYPTO_r_lock(CRYPTO_LOCK_RAND2);
		do_not_lock = !CRYPTO_THREADID_cmp(&locking_threadid, &cur);
		CRYPTO_r_unlock(CRYPTO_LOCK_RAND2);
		}
	else
		do_not_lock = 0;
	
	if (!do_not_lock)
		{
		CRYPTO_w_lock(CRYPTO_LOCK_RAND);
		
		/* prevent ssleay_rand_bytes() from trying to obtain the lock again */
		CRYPTO_w_lock(CRYPTO_LOCK_RAND2);
		CRYPTO_THREADID_cpy(&locking_threadid, &cur);
		CRYPTO_w_unlock(CRYPTO_LOCK_RAND2);
		crypto_lock_rand = 1;
		}
	
	if (!initialized)
		{
		RAND_poll();
		initialized = 1;
		}

	ret = entropy >= ENTROPY_NEEDED;

	if (!do_not_lock)
		{
		/* before unlocking, we must clear 'crypto_lock_rand' */
		crypto_lock_rand = 0;
		
		CRYPTO_w_unlock(CRYPTO_LOCK_RAND);
		}
	
	return ret;
	}