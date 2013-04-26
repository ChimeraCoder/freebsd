
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/iconv.h>
#include <sys/sysctl.h>
#include <ctype.h>
#ifndef APPLE
#include <dlfcn.h>
#endif
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <err.h>
#include <netsmb/smb_lib.h>

/*
 * prototype iconv* functions
 */
typedef void *iconv_t;

static iconv_t (*my_iconv_open)(const char *, const char *);
static size_t(*my_iconv)(iconv_t, const char **, size_t *, char **, size_t *);
static int(*my_iconv_close)(iconv_t);

u_char nls_lower[256];
u_char nls_upper[256];

static iconv_t nls_toext, nls_toloc;
static int iconv_loaded;
static void *iconv_lib;

int
nls_setlocale(const char *name)
{
	int i;

	if (setlocale(LC_CTYPE, name) == NULL) {
		warnx("can't set locale '%s'\n", name);
		return EINVAL;
	}
	for (i = 0; i < 256; i++) {
		nls_lower[i] = tolower(i);
		nls_upper[i] = toupper(i);
	}
	return 0;
}

int
nls_setrecode(const char *local, const char *external)
{
#ifdef APPLE
	return ENOENT;
#else
	iconv_t icd;

	if (iconv_loaded == 2)
		return ENOENT;
	else if (iconv_loaded == 0) {
		iconv_loaded++;
		iconv_lib = dlopen("libiconv.so", RTLD_LAZY | RTLD_GLOBAL);
		if (iconv_lib == NULL) {
			warn("Unable to load iconv library: %s\n", dlerror());
			iconv_loaded++;
			return ENOENT;
		}
		my_iconv_open = dlsym(iconv_lib, "iconv_open");
		my_iconv = dlsym(iconv_lib, "iconv");
		my_iconv_close = dlsym(iconv_lib, "iconv_close");
	}
	if (nls_toext)
		my_iconv_close(nls_toext);
	if (nls_toloc)
		my_iconv_close(nls_toloc);
	nls_toext = nls_toloc = (iconv_t)0;
	icd = my_iconv_open(external, local);
	if (icd == (iconv_t)-1)
		return errno;
	nls_toext = icd;
	icd = my_iconv_open(local, external);
	if (icd == (iconv_t)-1) {
		my_iconv_close(nls_toext);
		nls_toext = (iconv_t)0;
		return errno;
	}
	nls_toloc = icd;
	return 0;
#endif
}

char *
nls_str_toloc(char *dst, const char *src)
{
	char *p = dst;
	size_t inlen, outlen;

	if (!iconv_loaded)
		return strcpy(dst, src);

	if (nls_toloc == (iconv_t)0)
		return strcpy(dst, src);
	inlen = outlen = strlen(src);
	my_iconv(nls_toloc, NULL, NULL, &p, &outlen);
	while (my_iconv(nls_toloc, &src, &inlen, &p, &outlen) == -1) {
		*p++ = *src++;
		inlen--;
		outlen--;
	}
	*p = 0;
	return dst;
}

char *
nls_str_toext(char *dst, const char *src)
{
	char *p = dst;
	size_t inlen, outlen;

	if (!iconv_loaded)
		return strcpy(dst, src);

	if (nls_toext == (iconv_t)0)
		return strcpy(dst, src);
	inlen = outlen = strlen(src);
	my_iconv(nls_toext, NULL, NULL, &p, &outlen);
	while (my_iconv(nls_toext, &src, &inlen, &p, &outlen) == -1) {
		*p++ = *src++;
		inlen--;
		outlen--;
	}
	*p = 0;
	return dst;
}

void *
nls_mem_toloc(void *dst, const void *src, int size)
{
	char *p = dst;
	const char *s = src;
	size_t inlen, outlen;

	if (!iconv_loaded)
		return memcpy(dst, src, size);

	if (size == 0)
		return NULL;

	if (nls_toloc == (iconv_t)0)
		return memcpy(dst, src, size);
	inlen = outlen = size;
	my_iconv(nls_toloc, NULL, NULL, &p, &outlen);
	while (my_iconv(nls_toloc, &s, &inlen, &p, &outlen) == -1) {
		*p++ = *s++;
		inlen--;
		outlen--;
	}
	return dst;
}

void *
nls_mem_toext(void *dst, const void *src, int size)
{
	char *p = dst;
	const char *s = src;
	size_t inlen, outlen;

	if (size == 0)
		return NULL;

	if (!iconv_loaded || nls_toext == (iconv_t)0)
		return memcpy(dst, src, size);

	inlen = outlen = size;
	my_iconv(nls_toext, NULL, NULL, &p, &outlen);
	while (my_iconv(nls_toext, &s, &inlen, &p, &outlen) == -1) {
		*p++ = *s++;
		inlen--;
		outlen--;
	}
	return dst;
}

char *
nls_str_upper(char *dst, const char *src)
{
	char *p = dst;

	while (*src)
		*dst++ = toupper(*src++);
	*dst = 0;
	return p;
}

char *
nls_str_lower(char *dst, const char *src)
{
	char *p = dst;

	while (*src)
		*dst++ = tolower(*src++);
	*dst = 0;
	return p;
}