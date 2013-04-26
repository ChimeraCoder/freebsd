
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
 * Test program for wcsrtombs(), as specified by IEEE Std. 1003.1-2001 and
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
	wchar_t srcbuf[128];
	char dstbuf[128];
	wchar_t *src;
	mbstate_t s;

	/*
	 * C/POSIX locale.
	 */

	printf("1..1\n");

	/* Simple null terminated string. */
	wmemset(srcbuf, 0xcc, sizeof(srcbuf) / sizeof(*srcbuf));
	wcscpy(srcbuf, L"hello");
	memset(dstbuf, 0xcc, sizeof(dstbuf));
	src = srcbuf;
	memset(&s, 0, sizeof(s));
	assert(wcsrtombs(dstbuf, (const wchar_t **)&src, sizeof(dstbuf),
	    &s) == 5);
	assert(strcmp(dstbuf, "hello") == 0);
	assert((unsigned char)dstbuf[6] == 0xcc);
	assert(src == NULL);

	/* Not enough space in destination buffer. */
	wmemset(srcbuf, 0xcc, sizeof(srcbuf) / sizeof(*srcbuf));
	wcscpy(srcbuf, L"hello");
	memset(dstbuf, 0xcc, sizeof(dstbuf));
	src = srcbuf;
	memset(&s, 0, sizeof(s));
	assert(wcsrtombs(dstbuf, (const wchar_t **)&src, 4,
	    &s) == 4);
	assert(memcmp(dstbuf, "hell", 4) == 0);
	assert((unsigned char)dstbuf[5] == 0xcc);
	assert(src == srcbuf + 4);

	/* Null terminated string, internal dest. buffer */
	wmemset(srcbuf, 0xcc, sizeof(srcbuf) / sizeof(*srcbuf));
	wcscpy(srcbuf, L"hello");
	src = srcbuf;
	memset(&s, 0, sizeof(s));
	assert(wcsrtombs(NULL, (const wchar_t **)&src, sizeof(dstbuf),
	    &s) == 5);

	/* Null terminated string, internal state. */
	wmemset(srcbuf, 0xcc, sizeof(srcbuf) / sizeof(*srcbuf));
	wcscpy(srcbuf, L"hello");
	memset(dstbuf, 0xcc, sizeof(dstbuf));
	src = srcbuf;
	assert(wcsrtombs(dstbuf, (const wchar_t **)&src, sizeof(dstbuf),
	    NULL) == 5);
	assert(strcmp(dstbuf, "hello") == 0);
	assert((unsigned char)dstbuf[6] == 0xcc);
	assert(src == NULL);

	/* Null terminated string, internal state, internal dest. buffer. */
	wmemset(srcbuf, 0xcc, sizeof(srcbuf) / sizeof(*srcbuf));
	wcscpy(srcbuf, L"hello");
	src = srcbuf;
	assert(wcsrtombs(NULL, (const wchar_t **)&src, 0, NULL) == 5);

	/* Empty source buffer. */
	wmemset(srcbuf, 0xcc, sizeof(srcbuf) / sizeof(*srcbuf));
	srcbuf[0] = L'\0';
	memset(dstbuf, 0xcc, sizeof(dstbuf));
	src = srcbuf;
	memset(&s, 0, sizeof(s));
	assert(wcsrtombs(dstbuf, (const wchar_t **)&src, sizeof(dstbuf),
	    &s) == 0);
	assert(dstbuf[0] == L'\0');

	/* Zero length destination buffer. */
	wmemset(srcbuf, 0xcc, sizeof(srcbuf) / sizeof(*srcbuf));
	wcscpy(srcbuf, L"hello");
	memset(dstbuf, 0xcc, sizeof(dstbuf));
	src = srcbuf;
	memset(&s, 0, sizeof(s));
	assert(wcsrtombs(dstbuf, (const wchar_t **)&src, 0, &s) == 0);
	assert((unsigned char)dstbuf[0] == 0xcc);

	/*
	 * Japanese (EUC) locale.
	 */

	assert(strcmp(setlocale(LC_CTYPE, "ja_JP.eucJP"), "ja_JP.eucJP") == 0);
	assert(MB_CUR_MAX > 1);

	wmemset(srcbuf, 0xcc, sizeof(srcbuf) / sizeof(*srcbuf));
	srcbuf[0] = 0xA3C1;
	srcbuf[1] = 0x0020;
	srcbuf[2] = 0x0042;
	srcbuf[3] = 0x0020;
	srcbuf[4] = 0xA3C3;
	srcbuf[5] = 0x0000;
	memset(dstbuf, 0xcc, sizeof(dstbuf));
	src = srcbuf;
	memset(&s, 0, sizeof(s));
	assert(wcsrtombs(dstbuf, (const wchar_t **)&src, sizeof(dstbuf),
	    &s) == 7);
	assert(strcmp(dstbuf, "\xA3\xC1 B \xA3\xC3") == 0);
	assert((unsigned char)dstbuf[8] == 0xcc);
	assert(src == NULL);

	printf("ok 1 - wcsrtombs()\n");

	return (0);
}