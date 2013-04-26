
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
 * Test program for mbstowcs(), as specified by IEEE Std. 1003.1-2001 and
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
	char srcbuf[128];
	wchar_t dstbuf[128];

	/*
	 * C/POSIX locale.
	 */

	printf("1..1\n");

	/* Simple null terminated string. */
	memset(srcbuf, 0xcc, sizeof(srcbuf));
	strcpy(srcbuf, "hello");
	wmemset(dstbuf, 0xcccc, sizeof(dstbuf) / sizeof(*dstbuf));
	assert(mbstowcs(dstbuf, srcbuf, sizeof(dstbuf) / sizeof(*dstbuf)) == 5);
	assert(wcscmp(dstbuf, L"hello") == 0);
	assert(dstbuf[6] == 0xcccc);

	/* Not enough space in destination buffer. */
	memset(srcbuf, 0xcc, sizeof(srcbuf));
	strcpy(srcbuf, "hello");
	wmemset(dstbuf, 0xcccc, sizeof(dstbuf) / sizeof(*dstbuf));
	assert(mbstowcs(dstbuf, srcbuf, 4) == 4);
	assert(wmemcmp(dstbuf, L"hell", 4) == 0);
	assert(dstbuf[5] == 0xcccc);

	/* Null terminated string, internal dest. buffer (XSI extension) */
	memset(srcbuf, 0xcc, sizeof(srcbuf));
	strcpy(srcbuf, "hello");
	assert(mbstowcs(NULL, srcbuf, 0) == 5);

	/* Empty source buffer. */
	memset(srcbuf, 0xcc, sizeof(srcbuf));
	srcbuf[0] = '\0';
	wmemset(dstbuf, 0xcccc, sizeof(dstbuf) / sizeof(*dstbuf));
	assert(mbstowcs(dstbuf, srcbuf, 1) == 0);
	assert(dstbuf[0] == 0);
	assert(dstbuf[1] == 0xcccc);

	/* Zero length destination buffer. */
	memset(srcbuf, 0xcc, sizeof(srcbuf));
	strcpy(srcbuf, "hello");
	wmemset(dstbuf, 0xcccc, sizeof(dstbuf) / sizeof(*dstbuf));
	assert(mbstowcs(dstbuf, srcbuf, 0) == 0);
	assert(dstbuf[0] == 0xcccc);

	/*
	 * Japanese (EUC) locale.
	 */

	assert(strcmp(setlocale(LC_CTYPE, "ja_JP.eucJP"), "ja_JP.eucJP") == 0);
	assert(MB_CUR_MAX > 1);

	memset(srcbuf, 0xcc, sizeof(srcbuf));
	strcpy(srcbuf, "\xA3\xC1 B \xA3\xC3");
	wmemset(dstbuf, 0xcccc, sizeof(dstbuf) / sizeof(*dstbuf));
	assert(mbstowcs(dstbuf, srcbuf, sizeof(dstbuf) / sizeof(*dstbuf)) == 5);
	assert(dstbuf[0] == 0xA3C1 && dstbuf[1] == 0x20 && dstbuf[2] == 0x42 &&
	    dstbuf[3] == 0x20 && dstbuf[4] == 0xA3C3 && dstbuf[5] == 0);

	printf("ok 1 - mbstowcs()\n");

	return (0);
}