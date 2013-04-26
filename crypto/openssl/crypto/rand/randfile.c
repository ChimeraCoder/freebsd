
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

/* We need to define this to get macros like S_IFBLK and S_IFCHR */
#if !defined(OPENSSL_SYS_VXWORKS)
#define _XOPEN_SOURCE 500
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "e_os.h"
#include <openssl/crypto.h>
#include <openssl/rand.h>
#include <openssl/buffer.h>

#ifdef OPENSSL_SYS_VMS
#include <unixio.h>
#endif
#ifndef NO_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifndef OPENSSL_NO_POSIX_IO
# include <sys/stat.h>
#endif

#ifdef _WIN32
#define stat	_stat
#define chmod	_chmod
#define open	_open
#define fdopen	_fdopen
#endif

#undef BUFSIZE
#define BUFSIZE	1024
#define RAND_DATA 1024

#ifdef OPENSSL_SYS_VMS
/* This declaration is a nasty hack to get around vms' extension to fopen
 * for passing in sharing options being disabled by our /STANDARD=ANSI89 */
static FILE *(*const vms_fopen)(const char *, const char *, ...) =
    (FILE *(*)(const char *, const char *, ...))fopen;
#define VMS_OPEN_ATTRS "shr=get,put,upd,del","ctx=bin,stm","rfm=stm","rat=none","mrs=0"
#endif

/* #define RFILE ".rnd" - defined in ../../e_os.h */

/* Note that these functions are intended for seed files only.
 * Entropy devices and EGD sockets are handled in rand_unix.c */

int RAND_load_file(const char *file, long bytes)
	{
	/* If bytes >= 0, read up to 'bytes' bytes.
	 * if bytes == -1, read complete file. */

	MS_STATIC unsigned char buf[BUFSIZE];
#ifndef OPENSSL_NO_POSIX_IO
	struct stat sb;
#endif
	int i,ret=0,n;
	FILE *in;

	if (file == NULL) return(0);

#ifndef OPENSSL_NO_POSIX_IO
#ifdef PURIFY
	/* struct stat can have padding and unused fields that may not be
	 * initialized in the call to stat(). We need to clear the entire
	 * structure before calling RAND_add() to avoid complaints from
	 * applications such as Valgrind.
	 */
	memset(&sb, 0, sizeof(sb));
#endif
	if (stat(file,&sb) < 0) return(0);
	RAND_add(&sb,sizeof(sb),0.0);
#endif
	if (bytes == 0) return(ret);

#ifdef OPENSSL_SYS_VMS
	in=vms_fopen(file,"rb",VMS_OPEN_ATTRS);
#else
	in=fopen(file,"rb");
#endif
	if (in == NULL) goto err;
#if defined(S_IFBLK) && defined(S_IFCHR) && !defined(OPENSSL_NO_POSIX_IO)
	if (sb.st_mode & (S_IFBLK | S_IFCHR)) {
	  /* this file is a device. we don't want read an infinite number
	   * of bytes from a random device, nor do we want to use buffered
	   * I/O because we will waste system entropy. 
	   */
	  bytes = (bytes == -1) ? 2048 : bytes; /* ok, is 2048 enough? */
#ifndef OPENSSL_NO_SETVBUF_IONBF
	  setvbuf(in, NULL, _IONBF, 0); /* don't do buffered reads */
#endif /* ndef OPENSSL_NO_SETVBUF_IONBF */
	}
#endif
	for (;;)
		{
		if (bytes > 0)
			n = (bytes < BUFSIZE)?(int)bytes:BUFSIZE;
		else
			n = BUFSIZE;
		i=fread(buf,1,n,in);
		if (i <= 0) break;
#ifdef PURIFY
		RAND_add(buf,i,(double)i);
#else
		/* even if n != i, use the full array */
		RAND_add(buf,n,(double)i);
#endif
		ret+=i;
		if (bytes > 0)
			{
			bytes-=n;
			if (bytes <= 0) break;
			}
		}
	fclose(in);
	OPENSSL_cleanse(buf,BUFSIZE);
err:
	return(ret);
	}

int RAND_write_file(const char *file)
	{
	unsigned char buf[BUFSIZE];
	int i,ret=0,rand_err=0;
	FILE *out = NULL;
	int n;
#ifndef OPENSSL_NO_POSIX_IO
	struct stat sb;
	
	i=stat(file,&sb);
	if (i != -1) { 
#if defined(S_ISBLK) && defined(S_ISCHR)
	  if (S_ISBLK(sb.st_mode) || S_ISCHR(sb.st_mode)) {
	    /* this file is a device. we don't write back to it. 
	     * we "succeed" on the assumption this is some sort 
	     * of random device. Otherwise attempting to write to 
	     * and chmod the device causes problems.
	     */
	    return(1); 
	  }
#endif
	}
#endif

#if defined(O_CREAT) && !defined(OPENSSL_NO_POSIX_IO) && !defined(OPENSSL_SYS_VMS)
	{
#ifndef O_BINARY
#define O_BINARY 0
#endif
	/* chmod(..., 0600) is too late to protect the file,
	 * permissions should be restrictive from the start */
	int fd = open(file, O_WRONLY|O_CREAT|O_BINARY, 0600);
	if (fd != -1)
		out = fdopen(fd, "wb");
	}
#endif

#ifdef OPENSSL_SYS_VMS
	/* VMS NOTE: Prior versions of this routine created a _new_
	 * version of the rand file for each call into this routine, then
	 * deleted all existing versions named ;-1, and finally renamed
	 * the current version as ';1'. Under concurrent usage, this
	 * resulted in an RMS race condition in rename() which could
	 * orphan files (see vms message help for RMS$_REENT). With the
	 * fopen() calls below, openssl/VMS now shares the top-level
	 * version of the rand file. Note that there may still be
	 * conditions where the top-level rand file is locked. If so, this
	 * code will then create a new version of the rand file. Without
	 * the delete and rename code, this can result in ascending file
	 * versions that stop at version 32767, and this routine will then
	 * return an error. The remedy for this is to recode the calling
	 * application to avoid concurrent use of the rand file, or
	 * synchronize usage at the application level. Also consider
	 * whether or not you NEED a persistent rand file in a concurrent
	 * use situation. 
	 */

	out = vms_fopen(file,"rb+",VMS_OPEN_ATTRS);
	if (out == NULL)
		out = vms_fopen(file,"wb",VMS_OPEN_ATTRS);
#else
	if (out == NULL)
		out = fopen(file,"wb");
#endif
	if (out == NULL) goto err;

#ifndef NO_CHMOD
	chmod(file,0600);
#endif
	n=RAND_DATA;
	for (;;)
		{
		i=(n > BUFSIZE)?BUFSIZE:n;
		n-=BUFSIZE;
		if (RAND_bytes(buf,i) <= 0)
			rand_err=1;
		i=fwrite(buf,1,i,out);
		if (i <= 0)
			{
			ret=0;
			break;
			}
		ret+=i;
		if (n <= 0) break;
                }

	fclose(out);
	OPENSSL_cleanse(buf,BUFSIZE);
err:
	return (rand_err ? -1 : ret);
	}

const char *RAND_file_name(char *buf, size_t size)
	{
	char *s=NULL;
#ifdef __OpenBSD__
	struct stat sb;
#endif

	if (OPENSSL_issetugid() == 0)
		s=getenv("RANDFILE");
	if (s != NULL && *s && strlen(s) + 1 < size)
		{
		if (BUF_strlcpy(buf,s,size) >= size)
			return NULL;
		}
	else
		{
		if (OPENSSL_issetugid() == 0)
			s=getenv("HOME");
#ifdef DEFAULT_HOME
		if (s == NULL)
			{
			s = DEFAULT_HOME;
			}
#endif
		if (s && *s && strlen(s)+strlen(RFILE)+2 < size)
			{
			BUF_strlcpy(buf,s,size);
#ifndef OPENSSL_SYS_VMS
			BUF_strlcat(buf,"/",size);
#endif
			BUF_strlcat(buf,RFILE,size);
			}
		else
		  	buf[0] = '\0'; /* no file name */
		}

#ifdef __OpenBSD__
	/* given that all random loads just fail if the file can't be 
	 * seen on a stat, we stat the file we're returning, if it
	 * fails, use /dev/arandom instead. this allows the user to 
	 * use their own source for good random data, but defaults
	 * to something hopefully decent if that isn't available. 
	 */

	if (!buf[0])
		if (BUF_strlcpy(buf,"/dev/arandom",size) >= size) {
			return(NULL);
		}	
	if (stat(buf,&sb) == -1)
		if (BUF_strlcpy(buf,"/dev/arandom",size) >= size) {
			return(NULL);
		}	

#endif
	return(buf);
	}