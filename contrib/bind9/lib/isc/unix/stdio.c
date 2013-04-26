
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

/* $Id$ */

#include <config.h>

#include <errno.h>
#include <unistd.h>

#include <isc/stdio.h>
#include <isc/stat.h>

#include "errno2result.h"

isc_result_t
isc_stdio_open(const char *filename, const char *mode, FILE **fp) {
	FILE *f;

	f = fopen(filename, mode);
	if (f == NULL)
		return (isc__errno2result(errno));
	*fp = f;
	return (ISC_R_SUCCESS);
}

isc_result_t
isc_stdio_close(FILE *f) {
	int r;

	r = fclose(f);
	if (r == 0)
		return (ISC_R_SUCCESS);
	else
		return (isc__errno2result(errno));
}

isc_result_t
isc_stdio_seek(FILE *f, long offset, int whence) {
	int r;

	r = fseek(f, offset, whence);
	if (r == 0)
		return (ISC_R_SUCCESS);
	else
		return (isc__errno2result(errno));
}

isc_result_t
isc_stdio_read(void *ptr, size_t size, size_t nmemb, FILE *f, size_t *nret) {
	isc_result_t result = ISC_R_SUCCESS;
	size_t r;

	clearerr(f);
	r = fread(ptr, size, nmemb, f);
	if (r != nmemb) {
		if (feof(f))
			result = ISC_R_EOF;
		else
			result = isc__errno2result(errno);
	}
	if (nret != NULL)
		*nret = r;
	return (result);
}

isc_result_t
isc_stdio_write(const void *ptr, size_t size, size_t nmemb, FILE *f,
	       size_t *nret)
{
	isc_result_t result = ISC_R_SUCCESS;
	size_t r;

	clearerr(f);
	r = fwrite(ptr, size, nmemb, f);
	if (r != nmemb)
		result = isc__errno2result(errno);
	if (nret != NULL)
		*nret = r;
	return (result);
}

isc_result_t
isc_stdio_flush(FILE *f) {
	int r;

	r = fflush(f);
	if (r == 0)
		return (ISC_R_SUCCESS);
	else
		return (isc__errno2result(errno));
}

/*
 * OpenBSD has deprecated ENOTSUP in favor of EOPNOTSUPP.
 */
#if defined(EOPNOTSUPP) && !defined(ENOTSUP)
#define ENOTSUP EOPNOTSUPP
#endif

isc_result_t
isc_stdio_sync(FILE *f) {
	int r;

	r = fsync(fileno(f));
	/*
	 * fsync is not supported on sockets and pipes which
	 * result in EINVAL / ENOTSUP.
	 */
	if (r == 0 || errno == EINVAL || errno == ENOTSUP)
		return (ISC_R_SUCCESS);
	else
		return (isc__errno2result(errno));
}