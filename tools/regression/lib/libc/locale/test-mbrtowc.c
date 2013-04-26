
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
 * Test program for mbrtowc(), as specified by IEEE Std. 1003.1-2001 and
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
	wchar_t wc;
	char buf[MB_LEN_MAX + 1];

	/*
	 * C/POSIX locale.
	 */

	printf("1..1\n");

	assert(MB_CUR_MAX == 1);

	/* Null wide character, internal state. */
	memset(buf, 0xcc, sizeof(buf));
	buf[0] = 0;
	assert(mbrtowc(&wc, buf, 1, NULL) == 0);
	assert(wc == 0);

	/* Null wide character. */
	memset(&s, 0, sizeof(s));
	assert(mbrtowc(&wc, buf, 1, &s) == 0);
	assert(wc == 0);

	/* Latin letter A, internal state. */
	assert(mbrtowc(NULL, 0, 0, NULL) == 0);
	buf[0] = 'A';
	assert(mbrtowc(&wc, buf, 1, NULL) == 1);
	assert(wc == L'A');

	/* Latin letter A. */
	memset(&s, 0, sizeof(s));
	assert(mbrtowc(&wc, buf, 1, &s) == 1);
	assert(wc == L'A');

	/* Incomplete character sequence. */
	wc = L'z';
	memset(&s, 0, sizeof(s));
	assert(mbrtowc(&wc, buf, 0, &s) == (size_t)-2);
	assert(wc == L'z');

	/* Check that mbrtowc() doesn't access the buffer when n == 0. */
	wc = L'z';
	memset(&s, 0, sizeof(s));
	buf[0] = '\0';
	assert(mbrtowc(&wc, buf, 0, &s) == (size_t)-2);
	assert(wc == L'z');

	/*
	 * Japanese (EUC) locale.
	 */

	assert(strcmp(setlocale(LC_CTYPE, "ja_JP.eucJP"), "ja_JP.eucJP") == 0);
	assert(MB_CUR_MAX > 1);

	/* Null wide character, internal state. */
	assert(mbrtowc(NULL, 0, 0, NULL) == 0);
	memset(buf, 0xcc, sizeof(buf));
	buf[0] = 0;
	assert(mbrtowc(&wc, buf, 1, NULL) == 0);
	assert(wc == 0);

	/* Null wide character. */
	memset(&s, 0, sizeof(s));
	assert(mbrtowc(&wc, buf, 1, &s) == 0);
	assert(wc == 0);

	/* Latin letter A, internal state. */
	assert(mbrtowc(NULL, 0, 0, NULL) == 0);
	buf[0] = 'A';
	assert(mbrtowc(&wc, buf, 1, NULL) == 1);
	assert(wc == L'A');

	/* Latin letter A. */
	memset(&s, 0, sizeof(s));
	assert(mbrtowc(&wc, buf, 1, &s) == 1);
	assert(wc == L'A');

	/* Incomplete character sequence (zero length). */
	wc = L'z';
	memset(&s, 0, sizeof(s));
	assert(mbrtowc(&wc, buf, 0, &s) == (size_t)-2);
	assert(wc == L'z');

	/* Incomplete character sequence (truncated double-byte). */
	memset(buf, 0xcc, sizeof(buf));
	buf[0] = 0xa3;
	buf[1] = 0x00;
	memset(&s, 0, sizeof(s));
	wc = 0;
	assert(mbrtowc(&wc, buf, 1, &s) == (size_t)-2);

	/* Same as above, but complete. */
	buf[1] = 0xc1;
	memset(&s, 0, sizeof(s));
	wc = 0;
	assert(mbrtowc(&wc, buf, 2, &s) == 2);
	assert(wc == 0xa3c1);

	/* Test restarting behaviour. */
	memset(buf, 0xcc, sizeof(buf));
	buf[0] = 0xa3;
	memset(&s, 0, sizeof(s));
	wc = 0;
	assert(mbrtowc(&wc, buf, 1, &s) == (size_t)-2);
	assert(wc == 0);
	buf[0] = 0xc1;
	assert(mbrtowc(&wc, buf, 1, &s) == 1);
	assert(wc == 0xa3c1);

	printf("ok 1 - mbrtowc()\n");

	return (0);
}