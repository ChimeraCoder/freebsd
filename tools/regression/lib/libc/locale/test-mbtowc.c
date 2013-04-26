
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
 * Test program for mbtowc(), as specified by IEEE Std. 1003.1-2001 and
 * ISO/IEC 9899:1990.
 *
 * The function is tested with both the "C" ("POSIX") LC_CTYPE setting and
 * "ja_JP.eucJP". Other encodings are not tested.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <assert.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char *argv[])
{
	size_t len;
	wchar_t wc;
	char buf[MB_LEN_MAX + 1];

	/*
	 * C/POSIX locale.
	 */

	printf("1..1\n");

	assert(MB_CUR_MAX == 1);

	/* No shift states in C locale. */
	assert(mbtowc(NULL, NULL, 0) == 0);

	/* Null wide character. */
	wc = 0xcccc;
	memset(buf, 0, sizeof(buf));
	assert(mbtowc(&wc, buf, 1) == 0);
	assert(wc == 0);

	/* Latin letter A. */
	buf[0] = 'A';
	assert(mbtowc(&wc, buf, 1) == 1);
	assert(wc == L'A');

	/* Incomplete character sequence. */
	wc = L'z';
	buf[0] = '\0';
	assert(mbtowc(&wc, buf, 0) == -1);
	assert(wc == L'z');
	assert(mbtowc(NULL, NULL, 0) == 0);

	/*
	 * Japanese (EUC) locale.
	 */

	assert(strcmp(setlocale(LC_CTYPE, "ja_JP.eucJP"), "ja_JP.eucJP") == 0);
	assert(MB_CUR_MAX > 1);

	/* Null wide character */
	memset(buf, 0xcc, sizeof(buf));
	buf[0] = 0;
	wc = 0xcccc;
	assert(mbtowc(&wc, buf, 1) == 0);
	assert(wc == 0);

	/* Latin letter A. */
	buf[0] = 'A';
	assert(mbtowc(&wc, buf, 1) == 1);
	assert(wc == L'A');

	/* Incomplete character sequence (zero length). */
	wc = L'z';
	buf[0] = '\0';
	assert(mbtowc(&wc, buf, 0) == -1);
	assert(wc == L'z');
	assert(mbtowc(NULL, NULL, 0) == 0);

	/* Incomplete character sequence (truncated double-byte). */
	memset(buf, 0xcc, sizeof(buf));
	buf[0] = 0xa3;
	buf[1] = 0x00;
	wc = L'z';
	assert(mbtowc(&wc, buf, 1) == -1);
	assert(wc == L'z');
	assert(mbtowc(NULL, NULL, 0) == 0);

	/* Same as above, but complete. */
	buf[1] = 0xc1;
	assert(mbtowc(&wc, buf, 2) == 2);
	assert(wc == 0xa3c1);

	printf("ok 1 - mbtowc()\n");

	return (0);
}