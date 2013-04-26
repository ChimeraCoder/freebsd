
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
#include <openssl/lhash.h>
#include <openssl/crypto.h>
#include <openssl/buffer.h>
#include <openssl/err.h>

void ERR_print_errors_cb(int (*cb)(const char *str, size_t len, void *u),
			 void *u)
	{
	unsigned long l;
	char buf[256];
	char buf2[4096];
	const char *file,*data;
	int line,flags;
	unsigned long es;
	CRYPTO_THREADID cur;

	CRYPTO_THREADID_current(&cur);
	es=CRYPTO_THREADID_hash(&cur);
	while ((l=ERR_get_error_line_data(&file,&line,&data,&flags)) != 0)
		{
		ERR_error_string_n(l, buf, sizeof buf);
		BIO_snprintf(buf2, sizeof(buf2), "%lu:%s:%s:%d:%s\n", es, buf,
			file, line, (flags & ERR_TXT_STRING) ? data : "");
		if (cb(buf2, strlen(buf2), u) <= 0)
			break; /* abort outputting the error report */
		}
	}

#ifndef OPENSSL_NO_FP_API
static int print_fp(const char *str, size_t len, void *fp)
	{
	BIO bio;

	BIO_set(&bio,BIO_s_file());
	BIO_set_fp(&bio,fp,BIO_NOCLOSE);

	return BIO_printf(&bio, "%s", str);
	}
void ERR_print_errors_fp(FILE *fp)
	{
	ERR_print_errors_cb(print_fp, fp);
	}
#endif

static int print_bio(const char *str, size_t len, void *bp)
	{
	return BIO_write((BIO *)bp, str, len);
	}
void ERR_print_errors(BIO *bp)
	{
	ERR_print_errors_cb(print_bio, bp);
	}

	