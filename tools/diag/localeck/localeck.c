
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

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Try setlocale() for locale with given name.
 */

struct locdef {
	int		catid;
	const char	*catname;
} locales[_LC_LAST] = {
	{ LC_ALL,	"LC_ALL" },
	{ LC_COLLATE,	"LC_COLLATE" },
	{ LC_CTYPE,	"LC_CTYPE" },
	{ LC_MONETARY,	"LC_MONETARY" },
	{ LC_NUMERIC,	"LC_NUMERIC" },
	{ LC_TIME,	"LC_TIME" },
	{ LC_MESSAGES,	"LC_MESSAGES" }
};

int
main(int argc, char *argv[])
{
	int i, result;
	const char *localename;

	if (argc != 2) {
		(void)fprintf(stderr, "usage: localeck <locale_name>\n");
		exit(1);
	}

	localename = argv[1];
	result = 0;

	for (i = 0; i < _LC_LAST; i++) {
		if (setlocale(locales[i].catid, localename) == NULL) {
			printf("setlocale(%s, %s) failed\n", locales[i].catname,
			    localename);
			result++;
		}
	}
	return (result);
}