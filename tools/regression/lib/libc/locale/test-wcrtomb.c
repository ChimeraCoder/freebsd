
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

/*
 * Test program for wcrtomb(), as specified by IEEE Std. 1003.1-2001 and
 * ISO/IEC 9899:1999.
 *
 * The function is tested with both the "C" ("POSIX") LC_CTYPE setting and
 * "ja_JP.eucJP". Other encodings are not tested.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

int
main(int argc, char *argv[])
{
	mbstate_t s;
	size_t len;
	char buf[MB_LEN_MAX + 1];

	/*
	 * C/POSIX locale.
	 */

	printf("1..1\n");

	assert(MB_CUR_MAX == 1);

	/*
	 * If the buffer argument is NULL, wc is implicitly L'\0',
	 * wcrtomb() resets its internal state.
	 */
	assert(wcrtomb(NULL, L'\0', NULL) == 1);
	assert(wcrtomb(NULL, UCHAR_MAX + 1, NULL) == 1);

	/* Null wide character. */
	memset(&s, 0, sizeof(s));
	memset(buf, 0xcc, sizeof(buf));
	len = wcrtomb(buf, L'\0', &s);
	assert(len == 1);
	assert((unsigned char)buf[0] == 0 && (unsigned char)buf[1] == 0xcc);

	/* Latin letter A, internal state. */
	assert(wcrtomb(NULL, L'\0', NULL) == 1);
	assert(wcrtomb(NULL, L'A', NULL) == 1);

	/* Latin letter A. */
	memset(&s, 0, sizeof(s));
	memset(buf, 0xcc, sizeof(buf));
	len = wcrtomb(buf, L'A', &s);
	assert(len == 1);
	assert((unsigned char)buf[0] == 'A' && (unsigned char)buf[1] == 0xcc);

	/* Invalid code. */
	assert(wcrtomb(buf, UCHAR_MAX + 1, NULL) == (size_t)-1);
	assert(errno == EILSEQ);

	/*
	 * Japanese (EUC) locale.
	 */

	assert(strcmp(setlocale(LC_CTYPE, "ja_JP.eucJP"), "ja_JP.eucJP") == 0);
	assert(MB_CUR_MAX == 3);

	/*
	 * If the buffer argument is NULL, wc is implicitly L'\0',
	 * wcrtomb() resets its internal state.
	 */
	assert(wcrtomb(NULL, L'\0', NULL) == 1);

	/* Null wide character. */
	memset(&s, 0, sizeof(s));
	memset(buf, 0xcc, sizeof(buf));
	len = wcrtomb(buf, L'\0', &s);
	assert(len == 1);
	assert((unsigned char)buf[0] == 0 && (unsigned char)buf[1] == 0xcc);

	/* Latin letter A, internal state. */
	assert(wcrtomb(NULL, L'\0', NULL) == 1);
	assert(wcrtomb(NULL, L'A', NULL) == 1);

	/* Latin letter A. */
	memset(&s, 0, sizeof(s));
	memset(buf, 0xcc, sizeof(buf));
	len = wcrtomb(buf, L'A', &s);
	assert(len == 1);
	assert((unsigned char)buf[0] == 'A' && (unsigned char)buf[1] == 0xcc);

	/* Full width letter A. */
	memset(&s, 0, sizeof(s));
	memset(buf, 0xcc, sizeof(buf));
	len = wcrtomb(buf, 0xa3c1, &s);
	assert(len == 2);
	assert((unsigned char)buf[0] == 0xa3 &&
		(unsigned char)buf[1] == 0xc1 &&
		(unsigned char)buf[2] == 0xcc);

	printf("ok 1 - wcrtomb()\n");

	return (0);
}