
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
SM_IDSTR(id, "@(#)$Id: t-path.c,v 1.8 2001/09/11 04:04:49 gshapiro Exp $")

#include <string.h>
#include <sm/path.h>
#include <sm/test.h>

int
main(argc, argv)
	int argc;
	char **argv;
{
	char *r;

	sm_test_begin(argc, argv, "test path handling");

	SM_TEST(sm_path_isdevnull(SM_PATH_DEVNULL));
	r = "/dev/null";
	SM_TEST(sm_path_isdevnull(r));
	r = "/nev/dull";
	SM_TEST(!sm_path_isdevnull(r));
	r = "nul";
	SM_TEST(!sm_path_isdevnull(r));

	return sm_test_end();
}