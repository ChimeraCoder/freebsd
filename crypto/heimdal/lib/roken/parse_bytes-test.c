
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

#include <config.h>

#include "roken.h"
#include "parse_bytes.h"

static struct testcase {
    int canonicalp;
    int val;
    const char *def_unit;
    const char *str;
} tests[] = {
    {0, 0, NULL, "0 bytes"},
    {1, 0, NULL, "0"},
    {0, 1, NULL, "1"},
    {1, 1, NULL, "1 byte"},
    {0, 0, "kilobyte", "0"},
    {0, 1024, "kilobyte", "1"},
    {1, 1024, "kilobyte", "1 kilobyte"},
    {1, 1024 * 1024, NULL, "1 megabyte"},
    {0, 1025, NULL, "1 kilobyte 1"},
    {1, 1025, NULL, "1 kilobyte 1 byte"},
};

int
main(int argc, char **argv)
{
    int i;
    int ret = 0;

    for (i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
	char buf[256];
	int val = parse_bytes (tests[i].str, tests[i].def_unit);
	int len;

	if (val != tests[i].val) {
	    printf ("parse_bytes (%s, %s) = %d != %d\n",
		    tests[i].str,
		    tests[i].def_unit ? tests[i].def_unit : "none",
		    val, tests[i].val);
	    ++ret;
	}
	if (tests[i].canonicalp) {
	    len = unparse_bytes (tests[i].val, buf, sizeof(buf));
	    if (strcmp (tests[i].str, buf) != 0) {
		printf ("unparse_bytes (%d) = \"%s\" != \"%s\"\n",
			tests[i].val, buf, tests[i].str);
		++ret;
	    }
	}
    }
    if (ret) {
	printf ("%d errors\n", ret);
	return 1;
    } else
	return 0;
}