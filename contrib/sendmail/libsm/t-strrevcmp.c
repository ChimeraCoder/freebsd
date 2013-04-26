
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
SM_IDSTR(id, "@(#)$Id: t-strrevcmp.c,v 1.3 2001/09/11 04:04:49 gshapiro Exp $")

#include <sm/exc.h>
#include <sm/io.h>
#include <sm/string.h>
#include <sm/test.h>

int
main(argc, argv)
	int argc;
	char **argv;
{
	char *s1;
	char *s2;

	sm_test_begin(argc, argv, "test string compare");

	s1 = "equal";
	s2 = "equal";
	SM_TEST(sm_strrevcmp(s1, s2) == 0);

	s1 = "equal";
	s2 = "qual";
	SM_TEST(sm_strrevcmp(s1, s2) > 0);

	s1 = "qual";
	s2 = "equal";
	SM_TEST(sm_strrevcmp(s1, s2) < 0);

	s1 = "Equal";
	s2 = "equal";
	SM_TEST(sm_strrevcmp(s1, s2) < 0);

	s1 = "Equal";
	s2 = "equal";
	SM_TEST(sm_strrevcasecmp(s1, s2) == 0);

	s1 = "Equal";
	s2 = "eQuaL";
	SM_TEST(sm_strrevcasecmp(s1, s2) == 0);

	return sm_test_end();
}