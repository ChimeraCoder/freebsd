
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
 * Test program for mbrlen(), as specified by IEEE Std. 1003.1-2001 and
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

	/* Null wide character, internal state. */
	memset(buf, 0xcc, sizeof(buf));
	buf[0] = 0;
	assert(mbrlen(buf, 1, NULL) == 0);

	/* Null wide character. */
	memset(&s, 0, sizeof(s));
	assert(mbrlen(buf, 1, &s) == 0);

	/* Latin letter A, internal state. */
	assert(mbrlen(NULL, 0, NULL) == 0);
	buf[0] = 'A';
	assert(mbrlen(buf, 1, NULL) == 1);

	/* Latin letter A. */
	memset(&s, 0, sizeof(s));
	assert(mbrlen(buf, 1, &s) == 1);

	/* Incomplete character sequence. */
	memset(&s, 0, sizeof(s));
	assert(mbrlen(buf, 0, &s) == (size_t)-2);

	/*
	 * Japanese (EUC) locale.
	 */

	assert(strcmp(setlocale(LC_CTYPE, "ja_JP.eucJP"), "ja_JP.eucJP") == 0);
	assert(MB_CUR_MAX > 1);

	/* Null wide character, internal state. */
	assert(mbrlen(NULL, 0, NULL) == 0);
	memset(buf, 0xcc, sizeof(buf));
	buf[0] = 0;
	assert(mbrlen(buf, 1, NULL) == 0);

	/* Null wide character. */
	memset(&s, 0, sizeof(s));
	assert(mbrlen(buf, 1, &s) == 0);

	/* Latin letter A, internal state. */
	assert(mbrlen(NULL, 0, NULL) == 0);
	buf[0] = 'A';
	assert(mbrlen(buf, 1, NULL) == 1);

	/* Latin letter A. */
	memset(&s, 0, sizeof(s));
	assert(mbrlen(buf, 1, &s) == 1);

	/* Incomplete character sequence (zero length). */
	memset(&s, 0, sizeof(s));
	assert(mbrlen(buf, 0, &s) == (size_t)-2);

	/* Incomplete character sequence (truncated double-byte). */
	memset(buf, 0xcc, sizeof(buf));
	buf[0] = 0xa3;
	buf[1] = 0x00;
	memset(&s, 0, sizeof(s));
	assert(mbrlen(buf, 1, &s) == (size_t)-2);

	/* Same as above, but complete. */
	buf[1] = 0xc1;
	memset(&s, 0, sizeof(s));
	assert(mbrlen(buf, 2, &s) == 2);

	printf("ok 1 - mbrlen()\n");

	return (0);
}