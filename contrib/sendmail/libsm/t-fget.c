
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
SM_IDSTR(id, "@(#)$Id: t-fget.c,v 1.1 2013/03/12 15:24:50 ca Exp $")

#include <sm/io.h>
#include <sm/string.h>
#include <sm/test.h>
#include <errno.h>

void
check(char *msg, int l)
{
	SM_FILE_T *wfp, *rfp;
	char buf[256];
	size_t n;
	int r, i;
	static char fn[] = "tfget";

	wfp = sm_io_open(SmFtStdio, SM_TIME_DEFAULT, fn,
			   SM_IO_WRONLY_B, NULL);
	SM_TEST(wfp != NULL);
	for (i = 0; i < l; i++)
	{
		r = sm_io_putc(wfp, SM_TIME_DEFAULT, msg[i]);
		SM_TEST(r >= 0);
	}
	r = sm_io_close(wfp, SM_TIME_DEFAULT);
	SM_TEST(r == 0);

	rfp = sm_io_open(SmFtStdio, SM_TIME_DEFAULT, fn,
			   SM_IO_RDONLY_B, NULL);
	SM_TEST(rfp != NULL);
	n = sizeof(buf);
	r = sm_io_fgets(rfp, SM_TIME_DEFAULT, buf, n);
	if (l == 0)
	{
		SM_TEST(r == -1);
		SM_TEST(errno == 0);
	}
	else
	{
		SM_TEST(r == l);
		if (r != l)
			fprintf(stderr, "buf='%s', in='%s', r=%d, l=%d\n",
				buf, msg, r, l);
	}
	SM_TEST(memcmp(buf, msg, l) == 0);
	r = sm_io_close(rfp, SM_TIME_DEFAULT);
	SM_TEST(r == 0);
}


int
main(argc, argv)
	int argc;
	char **argv;
{
	char res[256];
	int l;

	sm_test_begin(argc, argv, "test fget");

	check("", strlen(""));
	check("\n", strlen("\n"));
	check("test\n", strlen("test\n"));

	l = snprintf(res, sizeof(res), "%c%s\n", '\0', "test ing");
	check(res, l);

	l = snprintf(res, sizeof(res), "%c%s%c\n", '\0', "test ing", '\0');
	check(res, l);

	l = snprintf(res, sizeof(res), "%c%s%c%s\n",
		'\0', "test ing", '\0', "eol");
	check(res, l);

	return sm_test_end();
}