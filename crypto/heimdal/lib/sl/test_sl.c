
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

#include "sl_locl.h"

struct {
    int ok;
    const char *line;
    int argc;
    const char *argv[4];
} lines[] = {
    { 1, "", 1, { "" } },
    { 1, "foo", 1, { "foo" } },
    { 1, "foo bar", 2, { "foo", "bar" }},
    { 1, "foo bar baz", 3, { "foo", "bar", "baz" }},
    { 1, "foobar baz", 2, { "foobar", "baz" }},
    { 1, " foo", 1, { "foo" } },
    { 1, "foo   ", 1, { "foo" } },
    { 1, " foo  ", 1, { "foo" } },
    { 1, " foo  bar", 2, { "foo", "bar" } },
    { 1, "foo\\ bar", 1, { "foo bar" } },
    { 1, "\"foo bar\"", 1, { "foo bar" } },
    { 1, "\"foo\\ bar\"", 1, { "foo bar" } },
    { 1, "\"foo\\\" bar\"", 1, { "foo\" bar" } },
    { 1, "\"\"f\"\"oo\"\"", 1, { "foo" } },
    { 1, "\"foobar\"baz", 1, { "foobarbaz" }},
    { 1, "foo\tbar baz", 3, { "foo", "bar", "baz" }},
    { 1, "\"foo bar\" baz", 2, { "foo bar", "baz" }},
    { 1, "\"foo bar baz\"", 1, { "foo bar baz" }},
    { 1, "\\\"foo bar baz", 3, { "\"foo", "bar", "baz" }},
    { 1, "\\ foo bar baz", 3, { " foo", "bar", "baz" }},
    { 0, "\\", 0, { "" }},
    { 0, "\"", 0, { "" }}
};

int
main(int argc, char **argv)
{
    int ret, i;

    for (i = 0; i < sizeof(lines)/sizeof(lines[0]); i++) {
	int j, rargc = 0;
	char **rargv = NULL;
	char *buf = strdup(lines[i].line);

	ret = sl_make_argv(buf, &rargc, &rargv);
	if (ret) {
	    if (!lines[i].ok)
		goto next;
	    errx(1, "sl_make_argv test %d failed", i);
	} else if (!lines[i].ok)
	    errx(1, "sl_make_argv passed test %d when it shouldn't", i);
	if (rargc != lines[i].argc)
	    errx(1, "result argc (%d) != should be argc (%d) for test %d",
		 rargc, lines[i].argc, i);
	for (j = 0; j < rargc; j++)
	    if (strcmp(rargv[j], lines[i].argv[j]) != 0)
		errx(1, "result argv (%s) != should be argv (%s) for test %d",
		     rargv[j], lines[i].argv[j], i);
    next:
	free(buf);
	free(rargv);
    }

    return 0;
}