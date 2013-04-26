
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
 * Test program for mbsnrtowcs().
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
	char srcbuf[128];
	wchar_t dstbuf[128];
	char *src;
	mbstate_t s;

	/*
	 * C/POSIX locale.
	 */

	printf("1..1\n");

	/* Simple null terminated string. */
	memset(srcbuf, 0xcc, sizeof(srcbuf));
	strcpy(srcbuf, "hello");
	wmemset(dstbuf, 0xcccc, sizeof(dstbuf) / sizeof(*dstbuf));
	src = srcbuf;
	memset(&s, 0, sizeof(s));
	assert(mbsnrtowcs(dstbuf, (const char **)&src, 6, sizeof(dstbuf) /
	    sizeof(*dstbuf), &s) == 5);
	assert(wcscmp(dstbuf, L"hello") == 0);
	assert(dstbuf[6] == 0xcccc);
	assert(src == NULL);

	/* Simple null terminated string, stopping early. */
	memset(srcbuf, 0xcc, sizeof(srcbuf));
	strcpy(srcbuf, "hello");
	wmemset(dstbuf, 0xcccc, sizeof(dstbuf) / sizeof(*dstbuf));
	src = srcbuf;
	memset(&s, 0, sizeof(s));
	assert(mbsnrtowcs(dstbuf, (const char **)&src, 4, sizeof(dstbuf) /
	    sizeof(*dstbuf), &s) == 4);
	assert(wmemcmp(dstbuf, L"hell", 4) == 0);
	assert(dstbuf[5] == 0xcccc);
	assert(src == srcbuf + 4);

	/* Not enough space in destination buffer. */
	memset(srcbuf, 0xcc, sizeof(srcbuf));
	strcpy(srcbuf, "hello");
	wmemset(dstbuf, 0xcccc, sizeof(dstbuf) / sizeof(*dstbuf));
	src = srcbuf;
	memset(&s, 0, sizeof(s));
	assert(mbsnrtowcs(dstbuf, (const char **)&src, 6, 4, &s) == 4);
	assert(wmemcmp(dstbuf, L"hell", 4) == 0);
	assert(dstbuf[5] == 0xcccc);
	assert(src == srcbuf + 4);

	/* Null terminated string, internal dest. buffer */
	memset(srcbuf, 0xcc, sizeof(srcbuf));
	strcpy(srcbuf, "hello");
	src = srcbuf;
	memset(&s, 0, sizeof(s));
	assert(mbsnrtowcs(NULL, (const char **)&src, 6, 0, &s) == 5);

	/* Null terminated string, internal dest. buffer, stopping early */
	memset(srcbuf, 0xcc, sizeof(srcbuf));
	strcpy(srcbuf, "hello");
	src = srcbuf;
	memset(&s, 0, sizeof(s));
	assert(mbsnrtowcs(NULL, (const char **)&src, 4, 0, &s) == 4);

	/* Null terminated string, internal state. */
	memset(srcbuf, 0xcc, sizeof(srcbuf));
	strcpy(srcbuf, "hello");
	wmemset(dstbuf, 0xcccc, sizeof(dstbuf) / sizeof(*dstbuf));
	src = srcbuf;
	assert(mbsnrtowcs(dstbuf, (const char **)&src, 6, sizeof(dstbuf) /
	    sizeof(*dstbuf), NULL) == 5);
	assert(wcscmp(dstbuf, L"hello") == 0);
	assert(dstbuf[6] == 0xcccc);
	assert(src == NULL);

	/* Null terminated string, internal state, internal dest. buffer. */
	memset(srcbuf, 0xcc, sizeof(srcbuf));
	strcpy(srcbuf, "hello");
	src = srcbuf;
	assert(mbsnrtowcs(NULL, (const char **)&src, 6, 0, NULL) == 5);

	/* Empty source buffer. */
	memset(srcbuf, 0xcc, sizeof(srcbuf));
	srcbuf[0] = '\0';
	src = srcbuf;
	memset(&s, 0, sizeof(s));
	wmemset(dstbuf, 0xcccc, sizeof(dstbuf) / sizeof(*dstbuf));
	assert(mbsnrtowcs(dstbuf, (const char **)&src, 1, 1, &s) == 0);
	assert(dstbuf[0] == 0);
	assert(dstbuf[1] == 0xcccc);
	assert(src == NULL);

	/* Zero length destination buffer. */
	memset(srcbuf, 0xcc, sizeof(srcbuf));
	strcpy(srcbuf, "hello");
	src = srcbuf;
	memset(&s, 0, sizeof(s));
	wmemset(dstbuf, 0xcccc, sizeof(dstbuf) / sizeof(*dstbuf));
	assert(mbsnrtowcs(dstbuf, (const char **)&src, 1, 0, &s) == 0);
	assert(dstbuf[0] == 0xcccc);
	assert(src == srcbuf);

	/* Zero length source buffer. */
	memset(srcbuf, 0xcc, sizeof(srcbuf));
	src = srcbuf;
	memset(&s, 0, sizeof(s));
	wmemset(dstbuf, 0xcccc, sizeof(dstbuf) / sizeof(*dstbuf));
	assert(mbsnrtowcs(dstbuf, (const char **)&src, 0, 1, &s) == 0);
	assert(dstbuf[0] == 0xcccc);
	assert(src == srcbuf);

	/*
	 * Japanese (EUC) locale.
	 */

	assert(strcmp(setlocale(LC_CTYPE, "ja_JP.eucJP"), "ja_JP.eucJP") == 0);
	assert(MB_CUR_MAX > 1);

	memset(srcbuf, 0xcc, sizeof(srcbuf));
	strcpy(srcbuf, "\xA3\xC1 B \xA3\xC3");
	src = srcbuf;
	memset(&s, 0, sizeof(s));
	wmemset(dstbuf, 0xcccc, sizeof(dstbuf) / sizeof(*dstbuf));
	assert(mbsnrtowcs(dstbuf, (const char **)&src, 8, sizeof(dstbuf) /
	    sizeof(*dstbuf), &s) == 5);
	assert(dstbuf[0] == 0xA3C1 && dstbuf[1] == 0x20 && dstbuf[2] == 0x42 &&
	    dstbuf[3] == 0x20 && dstbuf[4] == 0xA3C3 && dstbuf[5] == 0);
	assert(src == NULL);

	/* Partial character. */
	memset(srcbuf, 0xcc, sizeof(srcbuf));
	strcpy(srcbuf, "\xA3\xC1 B \xA3\xC3");
	src = srcbuf;
	memset(&s, 0, sizeof(s));
	wmemset(dstbuf, 0xcccc, sizeof(dstbuf) / sizeof(*dstbuf));
	assert(mbsnrtowcs(dstbuf, (const char **)&src, 6, sizeof(dstbuf) /
	    sizeof(*dstbuf), &s) == 4);
	assert(src == srcbuf + 6);
	assert(!mbsinit(&s));
	assert(mbsnrtowcs(dstbuf, (const char **)&src, 1, sizeof(dstbuf) /
	    sizeof(*dstbuf), &s) == 1);
	assert(src == srcbuf + 7);
	assert(mbsnrtowcs(dstbuf, (const char **)&src, 1, sizeof(dstbuf) /
	    sizeof(*dstbuf), &s) == 0);
	assert(src == NULL);

	printf("ok 1 - mbsnrtowcs()\n");

	return (0);
}