
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
 * Test program for wctype() and iswctype() as specified by
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
		int (*func)(wint_t);
	} cls[] = {
		{ "alnum", iswalnum },
		{ "alpha", iswalpha },
		{ "blank", iswblank },
		{ "cntrl", iswcntrl },
		{ "digit", iswdigit },
		{ "graph", iswgraph },
		{ "lower", iswlower },
		{ "print", iswprint },
		{ "punct", iswpunct },
		{ "space", iswspace },
		{ "upper", iswupper },
		{ "xdigit", iswxdigit }
	};

	printf("1..2\n");

	/*
	 * C/POSIX locale.
	 */
	for (i = 0; i < sizeof(cls) / sizeof(*cls); i++) {
		t = wctype(cls[i].name);
		assert(t != 0);
		for (j = 0; j < 256; j++)
			assert(cls[i].func(j) == iswctype(j, t));
	}
	t = wctype("elephant");
	assert(t == 0);
	for (i = 0; i < 256; i++)
		assert(iswctype(i, t) == 0);

	/*
	 * Japanese (EUC) locale.
	 */
	assert(strcmp(setlocale(LC_CTYPE, "ja_JP.eucJP"), "ja_JP.eucJP") == 0);
	for (i = 0; i < sizeof(cls) / sizeof(*cls); i++) {
		t = wctype(cls[i].name);
		assert(t != 0);
		for (j = 0; j < 65536; j++)
			assert(cls[i].func(j) == iswctype(j, t));
	}
	t = wctype("elephant");
	assert(t == 0);
	for (i = 0; i < 65536; i++)
		assert(iswctype(i, t) == 0);

	printf("ok 1 - iswctype()\n");
	printf("ok 2 - wctype()\n");

	return (0);
}