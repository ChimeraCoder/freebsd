
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

#include <sm/gen.h>
SM_IDSTR(id, "@(#)$Id: t-match.c,v 1.9 2001/09/11 04:04:49 gshapiro Exp $")

#include <sm/string.h>
#include <sm/io.h>
#include <sm/test.h>

#define try(str, pat, want) \
	got = sm_match(str, pat); \
	if (!SM_TEST(got == want)) \
		(void) sm_io_fprintf(smioout, SM_TIME_DEFAULT, \
			"sm_match(\"%s\", \"%s\") returns %s\n", \
			str, pat, got ? "true" : "false");

int
main(argc, argv)
	int argc;
	char **argv;
{
	bool got;

	sm_test_begin(argc, argv, "test sm_match");

	try("foo", "foo", true);
	try("foo", "bar", false);
	try("foo[bar", "foo[bar", true);
	try("foo[bar]", "foo[bar]", false);
	try("foob", "foo[bar]", true);
	try("a-b", "a[]-]b", true);
	try("abcde", "a*e", true);
	try("[", "[[]", true);
	try("c", "[a-z]", true);
	try("C", "[a-z]", false);
	try("F:sm.heap", "[!F]*", false);
	try("E:sm.err", "[!F]*", true);

	return sm_test_end();
}