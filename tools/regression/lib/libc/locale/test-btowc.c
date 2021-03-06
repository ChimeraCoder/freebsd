
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
 * Test program for btowc() and wctob() as specified by IEEE Std. 1003.1-2001
 * and ISO/IEC 9899:1999.
 *
 * The function is tested in the "C" and "ja_JP.eucJP" locales.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <assert.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

int
main(int argc, char *argv[])
{
	int i;

	printf("1..2\n");

	/*
	 * C/POSIX locale.
	 */
	assert(btowc(EOF) == WEOF);
	assert(wctob(WEOF) == EOF);
	for (i = 0; i < UCHAR_MAX; i++)
		assert(btowc(i) == (wchar_t)i && i == (int)wctob(i));

	/*
	 * Japanese (EUC) locale.
	 */

	assert(strcmp(setlocale(LC_CTYPE, "ja_JP.eucJP"), "ja_JP.eucJP") == 0);
	assert(MB_CUR_MAX > 1);
	assert(btowc('A') == L'A' && wctob(L'A') == 'A');
	assert(btowc(0xa3) == WEOF && wctob(0xa3c1) == EOF);

	printf("ok 1 - btowc()\n");
	printf("ok 2 - wctob()\n");

	return (0);
}