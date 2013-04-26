
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
#include <openssl/crypto.h>
#include "cryptlib.h"


static int allow_customize = 1;      /* we provide flexible functions for */
static int allow_customize_debug = 1;/* exchanging memory-related functions at
                                      * run-time, but this must be done
                                      * before any blocks are actually
                                      * allocated; or we'll run into huge
                                      * problems when malloc/free pairs
                                      * don't match etc. */



/* the following pointers may be changed as long as 'allow_customize' is set */

static void *(*malloc_func)(size_t)         = malloc;
static void *default_malloc_ex(size_t num, const char *file, int line)
	{ return malloc_func(num); }
static void *(*malloc_ex_func)(size_t, const char *file, int line)
        = default_malloc_ex;

static void *(*realloc_func)(void *, size_t)= realloc;
static void *default_realloc_ex(void *str, size_t num,
        const char *file, int line)
	{ return realloc_func(str,num); }
static void *(*realloc_ex_func)(void *, size_t, const char *file, int line)
        = default_realloc_ex;

static void (*free_func)(void *)            = free;

static void *(*malloc_locked_func)(size_t)  = malloc;
static void *default_malloc_locked_ex(size_t num, const char *file, int line)
	{ return malloc_locked_func(num); }
static void *(*malloc_locked_ex_func)(size_t, const char *file, int line)
        = default_malloc_locked_ex;

static void (*free_locked_func)(void *)     = free;



/* may be changed as long as 'allow_customize_debug' is set */
/* XXX use correct function pointer types */
#ifdef CRYPTO_MDEBUG
/* use default functions from mem_dbg.c */
static void (*malloc_debug_func)(void *,int,const char *,int,int)
	= CRYPTO_dbg_malloc;
static void (*realloc_debug_func)(void *,void *,int,const char *,int,int)
	= CRYPTO_dbg_realloc;
static void (*free_debug_func)(void *,int) = CRYPTO_dbg_free;
static void (*set_debug_options_func)(long) = CRYPTO_dbg_set_options;
static long (*get_debug_options_func)(void) = CRYPTO_dbg_get_options;
#else
/* applications can use CRYPTO_malloc_debug_init() to select above case
 * at run-time */
static void (*malloc_debug_func)(void *,int,const char *,int,int) = NULL;
static void (*realloc_debug_func)(void *,void *,int,const char *,int,int)
	= NULL;
static void (*free_debug_func)(void *,int) = NULL;
static void (*set_debug_options_func)(long) = NULL;
static long (*get_debug_options_func)(void) = NULL;
#endif

int CRYPTO_set_mem_functions(void *(*m)(size_t), void *(*r)(void *, size_t),
	void (*f)(void *))
	{
	/* Dummy call just to ensure OPENSSL_init() gets linked in */
	OPENSSL_init();
	if (!allow_customize)
		return 0;
	if ((m == 0) || (r == 0) || (f == 0))
		return 0;
	malloc_func=m; malloc_ex_func=default_malloc_ex;
	realloc_func=r; realloc_ex_func=default_realloc_ex;
	free_func=f;
	malloc_locked_func=m; malloc_locked_ex_func=default_malloc_locked_ex;
	free_locked_func=f;
	return 1;
	}

int CRYPTO_set_mem_ex_functions(
        void *(*m)(size_t,const char *,int),
        void *(*r)(void *, size_t,const char *,int),
	void (*f)(void *))
	{
	if (!allow_customize)
		return 0;
	if ((m == 0) || (r == 0) || (f == 0))
		return 0;
	malloc_func=0; malloc_ex_func=m;
	realloc_func=0; realloc_ex_func=r;
	free_func=f;
	malloc_locked_func=0; malloc_locked_ex_func=m;
	free_locked_func=f;
	return 1;
	}

int CRYPTO_set_locked_mem_functions(void *(*m)(size_t), void (*f)(void *))
	{
	if (!allow_customize)
		return 0;
	if ((m == NULL) || (f == NULL))
		return 0;
	malloc_locked_func=m; malloc_locked_ex_func=default_malloc_locked_ex;
	free_locked_func=f;
	return 1;
	}

int CRYPTO_set_locked_mem_ex_functions(
        void *(*m)(size_t,const char *,int),
        void (*f)(void *))
	{
	if (!allow_customize)
		return 0;
	if ((m == NULL) || (f == NULL))
		return 0;
	malloc_locked_func=0; malloc_locked_ex_func=m;
	free_func=f;
	return 1;
	}

int CRYPTO_set_mem_debug_functions(void (*m)(void *,int,const char *,int,int),
				   void (*r)(void *,void *,int,const char *,int,int),
				   void (*f)(void *,int),
				   void (*so)(long),
				   long (*go)(void))
	{
	if (!allow_customize_debug)
		return 0;
	OPENSSL_init();
	malloc_debug_func=m;
	realloc_debug_func=r;
	free_debug_func=f;
	set_debug_options_func=so;
	get_debug_options_func=go;
	return 1;
	}


void CRYPTO_get_mem_functions(void *(**m)(size_t), void *(**r)(void *, size_t),
	void (**f)(void *))
	{
	if (m != NULL) *m = (malloc_ex_func == default_malloc_ex) ? 
	                     malloc_func : 0;
	if (r != NULL) *r = (realloc_ex_func == default_realloc_ex) ? 
	                     realloc_func : 0;
	if (f != NULL) *f=free_func;
	}

void CRYPTO_get_mem_ex_functions(
        void *(**m)(size_t,const char *,int),
        void *(**r)(void *, size_t,const char *,int),
	void (**f)(void *))
	{
	if (m != NULL) *m = (malloc_ex_func != default_malloc_ex) ?
	                    malloc_ex_func : 0;
	if (r != NULL) *r = (realloc_ex_func != default_realloc_ex) ?
	                    realloc_ex_func : 0;
	if (f != NULL) *f=free_func;
	}

void CRYPTO_get_locked_mem_functions(void *(**m)(size_t), void (**f)(void *))
	{
	if (m != NULL) *m = (malloc_locked_ex_func == default_malloc_locked_ex) ? 
	                     malloc_locked_func : 0;
	if (f != NULL) *f=free_locked_func;
	}

void CRYPTO_get_locked_mem_ex_functions(
        void *(**m)(size_t,const char *,int),
        void (**f)(void *))
	{
	if (m != NULL) *m = (malloc_locked_ex_func != default_malloc_locked_ex) ?
	                    malloc_locked_ex_func : 0;
	if (f != NULL) *f=free_locked_func;
	}

void CRYPTO_get_mem_debug_functions(void (**m)(void *,int,const char *,int,int),
				    void (**r)(void *,void *,int,const char *,int,int),
				    void (**f)(void *,int),
				    void (**so)(long),
				    long (**go)(void))
	{
	if (m != NULL) *m=malloc_debug_func;
	if (r != NULL) *r=realloc_debug_func;
	if (f != NULL) *f=free_debug_func;
	if (so != NULL) *so=set_debug_options_func;
	if (go != NULL) *go=get_debug_options_func;
	}


void *CRYPTO_malloc_locked(int num, const char *file, int line)
	{
	void *ret = NULL;

	if (num <= 0) return NULL;

	allow_customize = 0;
	if (malloc_debug_func != NULL)
		{
		allow_customize_debug = 0;
		malloc_debug_func(NULL, num, file, line, 0);
		}
	ret = malloc_locked_ex_func(num,file,line);
#ifdef LEVITTE_DEBUG_MEM
	fprintf(stderr, "LEVITTE_DEBUG_MEM:         > 0x%p (%d)\n", ret, num);
#endif
	if (malloc_debug_func != NULL)
		malloc_debug_func(ret, num, file, line, 1);

#ifndef OPENSSL_CPUID_OBJ
        /* Create a dependency on the value of 'cleanse_ctr' so our memory
         * sanitisation function can't be optimised out. NB: We only do
         * this for >2Kb so the overhead doesn't bother us. */
        if(ret && (num > 2048))
	{	extern unsigned char cleanse_ctr;
		((unsigned char *)ret)[0] = cleanse_ctr;
	}
#endif

	return ret;
	}

void CRYPTO_free_locked(void *str)
	{
	if (free_debug_func != NULL)
		free_debug_func(str, 0);
#ifdef LEVITTE_DEBUG_MEM
	fprintf(stderr, "LEVITTE_DEBUG_MEM:         < 0x%p\n", str);
#endif
	free_locked_func(str);
	if (free_debug_func != NULL)
		free_debug_func(NULL, 1);
	}

void *CRYPTO_malloc(int num, const char *file, int line)
	{
	void *ret = NULL;

	if (num <= 0) return NULL;

	allow_customize = 0;
	if (malloc_debug_func != NULL)
		{
		allow_customize_debug = 0;
		malloc_debug_func(NULL, num, file, line, 0);
		}
	ret = malloc_ex_func(num,file,line);
#ifdef LEVITTE_DEBUG_MEM
	fprintf(stderr, "LEVITTE_DEBUG_MEM:         > 0x%p (%d)\n", ret, num);
#endif
	if (malloc_debug_func != NULL)
		malloc_debug_func(ret, num, file, line, 1);

#ifndef OPENSSL_CPUID_OBJ
        /* Create a dependency on the value of 'cleanse_ctr' so our memory
         * sanitisation function can't be optimised out. NB: We only do
         * this for >2Kb so the overhead doesn't bother us. */
        if(ret && (num > 2048))
	{	extern unsigned char cleanse_ctr;
                ((unsigned char *)ret)[0] = cleanse_ctr;
	}
#endif

	return ret;
	}
char *CRYPTO_strdup(const char *str, const char *file, int line)
	{
	char *ret = CRYPTO_malloc(strlen(str)+1, file, line);

	strcpy(ret, str);
	return ret;
	}

void *CRYPTO_realloc(void *str, int num, const char *file, int line)
	{
	void *ret = NULL;

	if (str == NULL)
		return CRYPTO_malloc(num, file, line);

	if (num <= 0) return NULL;

	if (realloc_debug_func != NULL)
		realloc_debug_func(str, NULL, num, file, line, 0);
	ret = realloc_ex_func(str,num,file,line);
#ifdef LEVITTE_DEBUG_MEM
	fprintf(stderr, "LEVITTE_DEBUG_MEM:         | 0x%p -> 0x%p (%d)\n", str, ret, num);
#endif
	if (realloc_debug_func != NULL)
		realloc_debug_func(str, ret, num, file, line, 1);

	return ret;
	}

void *CRYPTO_realloc_clean(void *str, int old_len, int num, const char *file,
			   int line)
	{
	void *ret = NULL;

	if (str == NULL)
		return CRYPTO_malloc(num, file, line);

	if (num <= 0) return NULL;

	/* We don't support shrinking the buffer. Note the memcpy that copies
	 * |old_len| bytes to the new buffer, below. */
	if (num < old_len) return NULL;

	if (realloc_debug_func != NULL)
		realloc_debug_func(str, NULL, num, file, line, 0);
	ret=malloc_ex_func(num,file,line);
	if(ret)
		{
		memcpy(ret,str,old_len);
		OPENSSL_cleanse(str,old_len);
		free_func(str);
		}
#ifdef LEVITTE_DEBUG_MEM
	fprintf(stderr,
		"LEVITTE_DEBUG_MEM:         | 0x%p -> 0x%p (%d)\n",
		str, ret, num);
#endif
	if (realloc_debug_func != NULL)
		realloc_debug_func(str, ret, num, file, line, 1);

	return ret;
	}

void CRYPTO_free(void *str)
	{
	if (free_debug_func != NULL)
		free_debug_func(str, 0);
#ifdef LEVITTE_DEBUG_MEM
	fprintf(stderr, "LEVITTE_DEBUG_MEM:         < 0x%p\n", str);
#endif
	free_func(str);
	if (free_debug_func != NULL)
		free_debug_func(NULL, 1);
	}

void *CRYPTO_remalloc(void *a, int num, const char *file, int line)
	{
	if (a != NULL) OPENSSL_free(a);
	a=(char *)OPENSSL_malloc(num);
	return(a);
	}

void CRYPTO_set_mem_debug_options(long bits)
	{
	if (set_debug_options_func != NULL)
		set_debug_options_func(bits);
	}

long CRYPTO_get_mem_debug_options(void)
	{
	if (get_debug_options_func != NULL)
		return get_debug_options_func();
	return 0;
	}