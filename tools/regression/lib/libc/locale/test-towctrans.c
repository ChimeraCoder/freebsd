
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
 * Test program for wctrans() and towctrans() as specified by
 * IEEE Std. 1003.1-2001 and ISO/IEC 9899:1999.
 *
 * The functions are tested in the "C" and "ja_JP.eucJP" locales.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <assert.h>
#include <locale.h>
#include <stdio.h>
#include <wchar.h>
#include <wctype.h>

int
main(int argc, char *argv[])
{
	wctype_t t;
	int i, j;
	struct {
		const char *name;
		wint_t (*func)(wint_t);
	} tran[] = {
		{ "tolower", towlower },
		{ "toupper", towupper },
	};

	printf("1..2\n");

	/*
	 * C/POSIX locale.
	 */
	for (i = 0; i < sizeof(tran) / sizeof(*tran); i++) {
		t = wctrans(tran[i].name);
		assert(t != 0);
		for (j = 0; j < 256; j++)
			assert(tran[i].func(j) == towctrans(j, t));
	}
	t = wctrans("elephant");
	assert(t == 0);
	for (i = 0; i < 256; i++)
		assert(towctrans(i, t) == i);

	/*
	 * Japanese (EUC) locale.
	 */
	assert(strcmp(setlocale(LC_CTYPE, "ja_JP.eucJP"), "ja_JP.eucJP") == 0);
	for (i = 0; i < sizeof(tran) / sizeof(*tran); i++) {
		t = wctrans(tran[i].name);
		assert(t != 0);
		for (j = 0; j < 65536; j++)
			assert(tran[i].func(j) == towctrans(j, t));
	}
	t = wctrans("elephant");
	assert(t == 0);
	for (i = 0; i < 65536; i++)
		assert(towctrans(i, t) == i);

	printf("ok 1 - towctrans()\n");
	printf("ok 2 - wctrans()\n");

	return (0);
}