
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
SM_IDSTR(id, "@(#)$Id: t-scanf.c,v 1.5 2001/11/13 00:51:28 ca Exp $")

#include <sm/limits.h>
#include <sm/io.h>
#include <sm/string.h>
#include <sm/test.h>
#include <sm/types.h>

int
main(argc, argv)
	int argc;
	char **argv;
{
	int i, d, h;
	char buf[128];
	char *r;

	sm_test_begin(argc, argv, "test scanf point stuff");
#if !SM_CONF_BROKEN_SIZE_T
	(void) sm_io_fprintf(smioout, SM_TIME_DEFAULT,
"If tests for \"h == 2\" fail, check whether size_t is signed on your OS.\n\
If that is the case, add -DSM_CONF_BROKEN_SIZE_T to confENVDEF\n\
and start over. Otherwise contact sendmail.org.\n");
#endif /* !SM_CONF_BROKEN_SIZE_T */

	d = 2;
	sm_snprintf(buf, sizeof(buf), "%d", d);
	r = "2";
	if (!SM_TEST(strcmp(buf, r) == 0))
		(void) sm_io_fprintf(smioerr, SM_TIME_DEFAULT,
				     "got %s instead\n", buf);

	i = sm_io_sscanf(buf, "%d", &h);
	SM_TEST(i == 1);
	SM_TEST(h == 2);

	d = 2;
	sm_snprintf(buf, sizeof(buf), "%d\n", d);
	r = "2\n";
	if (!SM_TEST(strcmp(buf, r) == 0))
		(void) sm_io_fprintf(smioerr, SM_TIME_DEFAULT,
				     "got %s instead\n", buf);

	i = sm_io_sscanf(buf, "%d", &h);
	SM_TEST(i == 1);
	SM_TEST(h == 2);

	return sm_test_end();
}