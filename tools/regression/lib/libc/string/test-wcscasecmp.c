
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <assert.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>

int
main(int argc, char *argv[])
{

	printf("1..6\n");

	setlocale(LC_CTYPE, "C");

	assert(wcscasecmp(L"", L"") == 0);
	assert(wcsncasecmp(L"", L"", 50) == 0);
	assert(wcsncasecmp(L"", L"", 0) == 0);
	printf("ok 1 - wcscasecmp\n");

	assert(wcscasecmp(L"abc", L"abc") == 0);
	assert(wcscasecmp(L"ABC", L"ABC") == 0);
	assert(wcscasecmp(L"abc", L"ABC") == 0);
	assert(wcscasecmp(L"ABC", L"abc") == 0);
	printf("ok 2 - wcscasecmp\n");

	assert(wcscasecmp(L"abc", L"xyz") < 0);
	assert(wcscasecmp(L"ABC", L"xyz") < 0);
	assert(wcscasecmp(L"abc", L"XYZ") < 0);
	assert(wcscasecmp(L"ABC", L"XYZ") < 0);
	assert(wcscasecmp(L"xyz", L"abc") > 0);
	assert(wcscasecmp(L"XYZ", L"abc") > 0);
	assert(wcscasecmp(L"xyz", L"ABC") > 0);
	assert(wcscasecmp(L"XYZ", L"ABC") > 0);
	printf("ok 3 - wcscasecmp\n");

	assert(wcscasecmp(L"abc", L"ABCD") < 0);
	assert(wcscasecmp(L"ABC", L"abcd") < 0);
	assert(wcscasecmp(L"abcd", L"ABC") > 0);
	assert(wcscasecmp(L"ABCD", L"abc") > 0);
	printf("ok 4 - wcscasecmp\n");

	assert(wcsncasecmp(L"abc", L"ABCD", 4) < 0);
	assert(wcsncasecmp(L"ABC", L"abcd", 4) < 0);
	assert(wcsncasecmp(L"abcd", L"ABC", 4) > 0);
	assert(wcsncasecmp(L"ABCD", L"abc", 4) > 0);
	assert(wcsncasecmp(L"abc", L"ABCD", 3) == 0);
	assert(wcsncasecmp(L"ABC", L"abcd", 3) == 0);
	printf("ok 5 - wcsncasecmp\n");

	assert(wcscasecmp(L"λ", L"Λ") != 0);
	setlocale(LC_CTYPE, "el_GR.UTF-8");
	assert(wcscasecmp(L"λ", L"Λ") == 0);
	assert(wcscasecmp(L"λ", L"Ω") < 0);
	assert(wcscasecmp(L"Ω", L"λ") > 0);
	printf("ok 6 - greek\n");

	exit(0);
}