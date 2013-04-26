
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
#include <roken.h>
#ifdef TEST_STRPFTIME
#include "strpftime-test.h"
#endif

enum { MAXSIZE = 26 };

static struct testcase {
    time_t t;
    struct {
	const char *format;
	const char *result;
    } vals[MAXSIZE];
} tests[] = {
    {0,
     {
	 {"%A", "Thursday"},
	 {"%a", "Thu"},
	 {"%B", "January"},
	 {"%b", "Jan"},
	 {"%C", "19"},
	 {"%d", "01"},
	 {"%e", " 1"},
	 {"%H", "00"},
	 {"%I", "12"},
	 {"%j", "001"},
	 {"%k", " 0"},
	 {"%l", "12"},
	 {"%M", "00"},
	 {"%m", "01"},
	 {"%n", "\n"},
	 {"%p", "AM"},
	 {"%S", "00"},
	 {"%t", "\t"},
	 {"%w", "4"},
	 {"%Y", "1970"},
	 {"%y", "70"},
	 {"%U", "00"},
	 {"%W", "00"},
	 {"%V", "01"},
	 {"%%", "%"},
	 {NULL, NULL}}
    },
    {90000,
     {
	 {"%A", "Friday"},
	 {"%a", "Fri"},
	 {"%B", "January"},
	 {"%b", "Jan"},
	 {"%C", "19"},
	 {"%d", "02"},
	 {"%e", " 2"},
	 {"%H", "01"},
	 {"%I", "01"},
	 {"%j", "002"},
	 {"%k", " 1"},
	 {"%l", " 1"},
	 {"%M", "00"},
	 {"%m", "01"},
	 {"%n", "\n"},
	 {"%p", "AM"},
	 {"%S", "00"},
	 {"%t", "\t"},
	 {"%w", "5"},
	 {"%Y", "1970"},
	 {"%y", "70"},
	 {"%U", "00"},
	 {"%W", "00"},
	 {"%V", "01"},
	 {"%%", "%"},
	 {NULL, NULL}
     }
    },
    {216306,
     {
	 {"%A", "Saturday"},
	 {"%a", "Sat"},
	 {"%B", "January"},
	 {"%b", "Jan"},
	 {"%C", "19"},
	 {"%d", "03"},
	 {"%e", " 3"},
	 {"%H", "12"},
	 {"%I", "12"},
	 {"%j", "003"},
	 {"%k", "12"},
	 {"%l", "12"},
	 {"%M", "05"},
	 {"%m", "01"},
	 {"%n", "\n"},
	 {"%p", "PM"},
	 {"%S", "06"},
	 {"%t", "\t"},
	 {"%w", "6"},
	 {"%Y", "1970"},
	 {"%y", "70"},
	 {"%U", "00"},
	 {"%W", "00"},
	 {"%V", "01"},
	 {"%%", "%"},
	 {NULL, NULL}
     }
    },
    {259200,
     {
	 {"%A", "Sunday"},
	 {"%a", "Sun"},
	 {"%B", "January"},
	 {"%b", "Jan"},
	 {"%C", "19"},
	 {"%d", "04"},
	 {"%e", " 4"},
	 {"%H", "00"},
	 {"%I", "12"},
	 {"%j", "004"},
	 {"%k", " 0"},
	 {"%l", "12"},
	 {"%M", "00"},
	 {"%m", "01"},
	 {"%n", "\n"},
	 {"%p", "AM"},
	 {"%S", "00"},
	 {"%t", "\t"},
	 {"%w", "0"},
	 {"%Y", "1970"},
	 {"%y", "70"},
	 {"%U", "01"},
	 {"%W", "00"},
	 {"%V", "01"},
	 {"%%", "%"},
	 {NULL, NULL}
     }
    },
    {915148800,
     {
	 {"%A", "Friday"},
	 {"%a", "Fri"},
	 {"%B", "January"},
	 {"%b", "Jan"},
	 {"%C", "19"},
	 {"%d", "01"},
	 {"%e", " 1"},
	 {"%H", "00"},
	 {"%I", "12"},
	 {"%j", "001"},
	 {"%k", " 0"},
	 {"%l", "12"},
	 {"%M", "00"},
	 {"%m", "01"},
	 {"%n", "\n"},
	 {"%p", "AM"},
	 {"%S", "00"},
	 {"%t", "\t"},
	 {"%w", "5"},
	 {"%Y", "1999"},
	 {"%y", "99"},
	 {"%U", "00"},
	 {"%W", "00"},
	 {"%V", "53"},
	 {"%%", "%"},
	 {NULL, NULL}}
    },
    {942161105,
     {

	 {"%A", "Tuesday"},
	 {"%a", "Tue"},
	 {"%B", "November"},
	 {"%b", "Nov"},
	 {"%C", "19"},
	 {"%d", "09"},
	 {"%e", " 9"},
	 {"%H", "15"},
	 {"%I", "03"},
	 {"%j", "313"},
	 {"%k", "15"},
	 {"%l", " 3"},
	 {"%M", "25"},
	 {"%m", "11"},
	 {"%n", "\n"},
	 {"%p", "PM"},
	 {"%S", "05"},
	 {"%t", "\t"},
	 {"%w", "2"},
	 {"%Y", "1999"},
	 {"%y", "99"},
	 {"%U", "45"},
	 {"%W", "45"},
	 {"%V", "45"},
	 {"%%", "%"},
	 {NULL, NULL}
     }
    }
};

int
main(int argc, char **argv)
{
    int i, j;
    int ret = 0;

    for (i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
	struct tm *tm;

	tm = gmtime (&tests[i].t);

	for (j = 0; tests[i].vals[j].format != NULL; ++j) {
	    char buf[128];
	    size_t len;
	    struct tm tm2;
	    char *ptr;

	    len = strftime (buf, sizeof(buf), tests[i].vals[j].format, tm);
	    if (len != strlen (buf)) {
		printf ("length of strftime(\"%s\") = %lu (\"%s\")\n",
			tests[i].vals[j].format, (unsigned long)len,
			buf);
		++ret;
		continue;
	    }
	    if (strcmp (buf, tests[i].vals[j].result) != 0) {
		printf ("result of strftime(\"%s\") = \"%s\" != \"%s\"\n",
			tests[i].vals[j].format, buf,
			tests[i].vals[j].result);
		++ret;
		continue;
	    }
	    memset (&tm2, 0, sizeof(tm2));
	    ptr = strptime (tests[i].vals[j].result,
			    tests[i].vals[j].format,
			    &tm2);
	    if (ptr == NULL || *ptr != '\0') {
		printf ("bad return value from strptime("
			"\"%s\", \"%s\")\n",
			tests[i].vals[j].result,
			tests[i].vals[j].format);
		++ret;
	    }
	    strftime (buf, sizeof(buf), tests[i].vals[j].format, &tm2);
	    if (strcmp (buf, tests[i].vals[j].result) != 0) {
		printf ("reverse of \"%s\" failed: \"%s\" vs \"%s\"\n",
			tests[i].vals[j].format,
			buf, tests[i].vals[j].result);
		++ret;
	    }
	}
    }
    {
	struct tm tm;
	memset(&tm, 0, sizeof(tm));
	strptime ("200505", "%Y%m", &tm);
	if (tm.tm_year != 105)
	    ++ret;
	if (tm.tm_mon != 4)
	    ++ret;
    }
    if (ret) {
	printf ("%d errors\n", ret);
	return 1;
    } else
	return 0;
}