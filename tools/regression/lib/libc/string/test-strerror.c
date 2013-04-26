
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

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tap.h>

int
main(void)
{
	char buf[64];
	char *sret;
	int iret;

	plan_tests(27);

	/*
	 * strerror() failure tests.
	 */
	errno = 0;
	sret = strerror(INT_MAX);
	snprintf(buf, sizeof(buf), "Unknown error: %d", INT_MAX);
	ok1(strcmp(sret, buf) == 0);
	ok1(errno == EINVAL);

	/*
	 * strerror() success tests.
	 */
	errno = 0;
	sret = strerror(0);
	ok1(strcmp(sret, "No error: 0") == 0);
	ok1(errno == 0);

	errno = 0;
	sret = strerror(EPERM);
	ok1(strcmp(sret, "Operation not permitted") == 0);
	ok1(errno == 0);

	errno = 0;
	sret = strerror(EPFNOSUPPORT);
	ok1(strcmp(sret, "Protocol family not supported") == 0);
	ok1(errno == 0);

	errno = 0;
	sret = strerror(ELAST);
	ok1(errno == 0);

	/*
	 * strerror_r() failure tests.
	 */
	memset(buf, '*', sizeof(buf));
	iret = strerror_r(-1, buf, sizeof(buf));
	ok1(strcmp(buf, "Unknown error: -1") == 0);
	ok1(iret == EINVAL);

	memset(buf, '*', sizeof(buf));
	/* One byte too short. */
	iret = strerror_r(EPERM, buf, strlen("Operation not permitted"));
	ok1(strcmp(buf, "Operation not permitte") == 0);
	ok1(iret == ERANGE);

	memset(buf, '*', sizeof(buf));
	/* One byte too short. */
	iret = strerror_r(-1, buf, strlen("Unknown error: -1"));
	ok1(strcmp(buf, "Unknown error: -") == 0);
	ok1(iret == EINVAL);

	memset(buf, '*', sizeof(buf));
	/* Two bytes too short. */
	iret = strerror_r(-2, buf, strlen("Unknown error: -2") - 1);
	ok1(strcmp(buf, "Unknown error: ") == 0);
	ok1(iret == EINVAL);

	memset(buf, '*', sizeof(buf));
	/* Three bytes too short. */
	iret = strerror_r(-2, buf, strlen("Unknown error: -2") - 2);
	ok1(strcmp(buf, "Unknown error:") == 0);
	ok1(iret == EINVAL);

	memset(buf, '*', sizeof(buf));
	/* One byte too short. */
	iret = strerror_r(12345, buf, strlen("Unknown error: 12345"));
	ok1(strcmp(buf, "Unknown error: 1234") == 0);
	ok1(iret == EINVAL);

	/*
	 * strerror_r() success tests.
	 */
	memset(buf, '*', sizeof(buf));
	iret = strerror_r(0, buf, sizeof(buf));
	ok1(strcmp(buf, "No error: 0") == 0);
	ok1(iret == 0);

	memset(buf, '*', sizeof(buf));
	iret = strerror_r(EDEADLK, buf, sizeof(buf));
	ok1(strcmp(buf, "Resource deadlock avoided") == 0);
	ok1(iret == 0);

	memset(buf, '*', sizeof(buf));
	iret = strerror_r(EPROCLIM, buf, sizeof(buf));
	ok1(strcmp(buf, "Too many processes") == 0);
	ok1(iret == 0);

	return exit_status();
}