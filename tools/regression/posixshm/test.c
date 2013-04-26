
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <stdarg.h>
#include <stdio.h>

#include "test.h"

static int test_index;
static struct regression_test *test;
static int test_acknowleged;

SET_DECLARE(regression_tests_set, struct regression_test);

/*
 * Outputs a test summary of the following:
 *
 * <status> <test #> [name] [# <fmt> [fmt args]]
 */
static void
vprint_status(const char *status, const char *fmt, va_list ap)
{

	printf("%s %d", status, test_index);
	if (test->rt_name)
		printf(" - %s", test->rt_name);
	if (fmt) {
		printf(" # ");
		vprintf(fmt, ap);
	}
	printf("\n");
	test_acknowleged = 1;
}

static void
print_status(const char *status, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vprint_status(status, fmt, ap);
	va_end(ap);
}

void
pass(void)
{

	print_status("ok", NULL);
}

void
fail(void)
{

	print_status("not ok", NULL);
}

void
fail_err(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vprint_status("not ok", fmt, ap);
	va_end(ap);	
}

void
skip(const char *reason)
{

	print_status("ok", "skip %s", reason);
}

void
todo(const char *reason)
{

	print_status("not ok", "TODO %s", reason);
}

void
run_tests(void)
{
	struct regression_test **testp;

	printf("1..%td\n", SET_COUNT(regression_tests_set));
	test_index = 1;
	SET_FOREACH(testp, regression_tests_set) {
		test_acknowleged = 0;
		test = *testp;
		test->rt_function();
		if (!test_acknowleged)
			print_status("not ok", "unknown status");
		test_index++;
	}
}