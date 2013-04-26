
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
SM_IDSTR(id, "@(#)$Id: t-fopen.c,v 1.9 2002/02/06 23:57:45 ca Exp $")

#include <fcntl.h>
#include <sm/io.h>
#include <sm/test.h>

/* ARGSUSED0 */
int
main(argc, argv)
	int argc;
	char *argv[];
{
	int m, r;
	SM_FILE_T *out;

	sm_test_begin(argc, argv, "test sm_io_fopen");
	out = sm_io_fopen("foo", O_WRONLY|O_APPEND|O_CREAT, 0666);
	SM_TEST(out != NULL);
	if (out != NULL)
	{
		(void) sm_io_fprintf(out, SM_TIME_DEFAULT, "foo\n");
		r = sm_io_getinfo(out, SM_IO_WHAT_MODE, &m);
		SM_TEST(r == 0);
		SM_TEST(m == SM_IO_WRONLY);
		sm_io_close(out, SM_TIME_DEFAULT);
	}
	return sm_test_end();
}