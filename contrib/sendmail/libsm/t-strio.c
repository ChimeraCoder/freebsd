
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
SM_IDSTR(id, "@(#)$Id: t-strio.c,v 1.11 2001/09/11 04:04:49 gshapiro Exp $")
#include <sm/string.h>
#include <sm/io.h>
#include <sm/test.h>

int
main(argc, argv)
	int argc;
	char *argv[];
{
	char buf[20];
	char *r;
	SM_FILE_T f;

	sm_test_begin(argc, argv, "test strio");
	(void) memset(buf, '.', 20);
	sm_strio_init(&f, buf, 10);
	(void) sm_io_fprintf(&f, SM_TIME_DEFAULT, "foobarbazoom");
	sm_io_flush(&f, SM_TIME_DEFAULT);
	r = "foobarbaz";
	SM_TEST(strcmp(buf, r) == 0);
	return sm_test_end();
}