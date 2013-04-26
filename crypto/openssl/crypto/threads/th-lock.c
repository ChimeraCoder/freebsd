
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
#include <errno.h>
#ifdef LINUX
#include <typedefs.h>
#endif
#ifdef OPENSSL_SYS_WIN32
#include <windows.h>
#endif
#ifdef SOLARIS
#include <synch.h>
#include <thread.h>
#endif
#ifdef IRIX
#include <ulocks.h>
#include <sys/prctl.h>
#endif
#ifdef PTHREADS
#include <pthread.h>
#endif
#include <openssl/lhash.h>
#include <openssl/crypto.h>
#include <openssl/buffer.h>
#include "../../e_os.h"
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

void CRYPTO_thread_setup(void);
void CRYPTO_thread_cleanup(void);

static void irix_locking_callback(int mode,int type,char *file,int line);
static void solaris_locking_callback(int mode,int type,char *file,int line);
static void win32_locking_callback(int mode,int type,char *file,int line);
static void pthreads_locking_callback(int mode,int type,char *file,int line);

static unsigned long irix_thread_id(void );
static unsigned long solaris_thread_id(void );
static unsigned long pthreads_thread_id(void );

/* usage:
 * CRYPTO_thread_setup();
 * application code
 * CRYPTO_thread_cleanup();
 */

#define THREAD_STACK_SIZE (16*1024)

#ifdef OPENSSL_SYS_WIN32

static HANDLE *lock_cs;

void CRYPTO_thread_setup(void)
	{
	int i;

	lock_cs=OPENSSL_malloc(CRYPTO_num_locks() * sizeof(HANDLE));
	for (i=0; i<CRYPTO_num_locks(); i++)
		{
		lock_cs[i]=CreateMutex(NULL,FALSE,NULL);
		}

	CRYPTO_set_locking_callback((void (*)(int,int,char *,int))win32_locking_callback);
	/* id callback defined */
	return(1);
	}

static void CRYPTO_thread_cleanup(void)
	{
	int i;

	CRYPTO_set_locking_callback(NULL);
	for (i=0; i<CRYPTO_num_locks(); i++)
		CloseHandle(lock_cs[i]);
	OPENSSL_free(lock_cs);
	}

void win32_locking_callback(int mode, int type, char *file, int line)
	{
	if (mode & CRYPTO_LOCK)
		{
		WaitForSingleObject(lock_cs[type],INFINITE);
		}
	else
		{
		ReleaseMutex(lock_cs[type]);
		}
	}

#endif /* OPENSSL_SYS_WIN32 */

#ifdef SOLARIS

#define USE_MUTEX

#ifdef USE_MUTEX
static mutex_t *lock_cs;
#else
static rwlock_t *lock_cs;
#endif
static long *lock_count;

void CRYPTO_thread_setup(void)
	{
	int i;

#ifdef USE_MUTEX
	lock_cs=OPENSSL_malloc(CRYPTO_num_locks() * sizeof(mutex_t));
#else
	lock_cs=OPENSSL_malloc(CRYPTO_num_locks() * sizeof(rwlock_t));
#endif
	lock_count=OPENSSL_malloc(CRYPTO_num_locks() * sizeof(long));
	for (i=0; i<CRYPTO_num_locks(); i++)
		{
		lock_count[i]=0;
#ifdef USE_MUTEX
		mutex_init(&(lock_cs[i]),USYNC_THREAD,NULL);
#else
		rwlock_init(&(lock_cs[i]),USYNC_THREAD,NULL);
#endif
		}

	CRYPTO_set_id_callback((unsigned long (*)())solaris_thread_id);
	CRYPTO_set_locking_callback((void (*)())solaris_locking_callback);
	}

void CRYPTO_thread_cleanup(void)
	{
	int i;

	CRYPTO_set_locking_callback(NULL);
	for (i=0; i<CRYPTO_num_locks(); i++)
		{
#ifdef USE_MUTEX
		mutex_destroy(&(lock_cs[i]));
#else
		rwlock_destroy(&(lock_cs[i]));
#endif
		}
	OPENSSL_free(lock_cs);
	OPENSSL_free(lock_count);
	}

void solaris_locking_callback(int mode, int type, char *file, int line)
	{
#if 0
	fprintf(stderr,"thread=%4d mode=%s lock=%s %s:%d\n",
		CRYPTO_thread_id(),
		(mode&CRYPTO_LOCK)?"l":"u",
		(type&CRYPTO_READ)?"r":"w",file,line);
#endif

#if 0
	if (CRYPTO_LOCK_SSL_CERT == type)
		fprintf(stderr,"(t,m,f,l) %ld %d %s %d\n",
			CRYPTO_thread_id(),
			mode,file,line);
#endif
	if (mode & CRYPTO_LOCK)
		{
#ifdef USE_MUTEX
		mutex_lock(&(lock_cs[type]));
#else
		if (mode & CRYPTO_READ)
			rw_rdlock(&(lock_cs[type]));
		else
			rw_wrlock(&(lock_cs[type]));
#endif
		lock_count[type]++;
		}
	else
		{
#ifdef USE_MUTEX
		mutex_unlock(&(lock_cs[type]));
#else
		rw_unlock(&(lock_cs[type]));
#endif
		}
	}

unsigned long solaris_thread_id(void)
	{
	unsigned long ret;

	ret=(unsigned long)thr_self();
	return(ret);
	}
#endif /* SOLARIS */

#ifdef IRIX
/* I don't think this works..... */

static usptr_t *arena;
static usema_t **lock_cs;

void CRYPTO_thread_setup(void)
	{
	int i;
	char filename[20];

	strcpy(filename,"/tmp/mttest.XXXXXX");
	mktemp(filename);

	usconfig(CONF_STHREADIOOFF);
	usconfig(CONF_STHREADMALLOCOFF);
	usconfig(CONF_INITUSERS,100);
	usconfig(CONF_LOCKTYPE,US_DEBUGPLUS);
	arena=usinit(filename);
	unlink(filename);

	lock_cs=OPENSSL_malloc(CRYPTO_num_locks() * sizeof(usema_t *));
	for (i=0; i<CRYPTO_num_locks(); i++)
		{
		lock_cs[i]=usnewsema(arena,1);
		}

	CRYPTO_set_id_callback((unsigned long (*)())irix_thread_id);
	CRYPTO_set_locking_callback((void (*)())irix_locking_callback);
	}

void CRYPTO_thread_cleanup(void)
	{
	int i;

	CRYPTO_set_locking_callback(NULL);
	for (i=0; i<CRYPTO_num_locks(); i++)
		{
		char buf[10];

		sprintf(buf,"%2d:",i);
		usdumpsema(lock_cs[i],stdout,buf);
		usfreesema(lock_cs[i],arena);
		}
	OPENSSL_free(lock_cs);
	}

void irix_locking_callback(int mode, int type, char *file, int line)
	{
	if (mode & CRYPTO_LOCK)
		{
		uspsema(lock_cs[type]);
		}
	else
		{
		usvsema(lock_cs[type]);
		}
	}

unsigned long irix_thread_id(void)
	{
	unsigned long ret;

	ret=(unsigned long)getpid();
	return(ret);
	}
#endif /* IRIX */

/* Linux and a few others */
#ifdef PTHREADS

static pthread_mutex_t *lock_cs;
static long *lock_count;

void CRYPTO_thread_setup(void)
	{
	int i;

	lock_cs=OPENSSL_malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
	lock_count=OPENSSL_malloc(CRYPTO_num_locks() * sizeof(long));
	for (i=0; i<CRYPTO_num_locks(); i++)
		{
		lock_count[i]=0;
		pthread_mutex_init(&(lock_cs[i]),NULL);
		}

	CRYPTO_set_id_callback((unsigned long (*)())pthreads_thread_id);
	CRYPTO_set_locking_callback((void (*)())pthreads_locking_callback);
	}

void thread_cleanup(void)
	{
	int i;

	CRYPTO_set_locking_callback(NULL);
	for (i=0; i<CRYPTO_num_locks(); i++)
		{
		pthread_mutex_destroy(&(lock_cs[i]));
		}
	OPENSSL_free(lock_cs);
	OPENSSL_free(lock_count);
	}

void pthreads_locking_callback(int mode, int type, char *file,
	     int line)
      {
#if 0
	fprintf(stderr,"thread=%4d mode=%s lock=%s %s:%d\n",
		CRYPTO_thread_id(),
		(mode&CRYPTO_LOCK)?"l":"u",
		(type&CRYPTO_READ)?"r":"w",file,line);
#endif
#if 0
	if (CRYPTO_LOCK_SSL_CERT == type)
		fprintf(stderr,"(t,m,f,l) %ld %d %s %d\n",
		CRYPTO_thread_id(),
		mode,file,line);
#endif
	if (mode & CRYPTO_LOCK)
		{
		pthread_mutex_lock(&(lock_cs[type]));
		lock_count[type]++;
		}
	else
		{
		pthread_mutex_unlock(&(lock_cs[type]));
		}
	}

unsigned long pthreads_thread_id(void)
	{
	unsigned long ret;

	ret=(unsigned long)pthread_self();
	return(ret);
	}

#endif /* PTHREADS */