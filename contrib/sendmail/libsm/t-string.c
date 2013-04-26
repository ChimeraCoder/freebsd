
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
SM_IDSTR(id, "@(#)$Id: t-string.c,v 1.11 2001/09/11 04:04:49 gshapiro Exp $")

#include <sm/exc.h>
#include <sm/io.h>
#include <sm/string.h>
#include <sm/test.h>

int
main(argc, argv)
	int argc;
	char **argv;
{
	char *s;
	char buf[4096];
	char foo[4];
	char *r;
	int n;

	sm_test_begin(argc, argv, "test string utilities");

	s = sm_stringf_x("%.3s%03d", "foobar", 42);
	r = "foo042";
	SM_TEST(strcmp(s, r) == 0);

	s = sm_stringf_x("+%*x+", 2000, 0xCAFE);
	sm_snprintf(buf, 4096, "+%*x+", 2000, 0xCAFE);
	SM_TEST(strcmp(s, buf) == 0);

	foo[3] = 1;
	n = sm_snprintf(foo, sizeof(foo), "foobar%dbaz", 42);
	SM_TEST(n == 11);
	r = "foo";
	SM_TEST(strcmp(foo, r) == 0);

	return sm_test_end();
}