
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

#include <sys/wait.h>

#include <err.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cap_test.h"

/* Initialize a named test. Requires test_NAME() function to be declared. */
#define	TEST_INIT(name)	{ #name, test_##name, FAILED }

/* All of the tests that can be run. */
struct test all_tests[] = {
	TEST_INIT(capmode),
	TEST_INIT(capabilities),
	TEST_INIT(fcntl),
	TEST_INIT(pdfork),
	TEST_INIT(pdkill),
	TEST_INIT(relative),
	TEST_INIT(sysctl),
};
int test_count = sizeof(all_tests) / sizeof(struct test);

int
main(int argc, char *argv[])
{

	/*
	 * If no tests have been specified at the command line, run them all.
	 */
	if (argc == 1) {
		printf("1..%d\n", test_count);

		for (int i = 0; i < test_count; i++)
			execute(i + 1, all_tests + i);
		return (0);
	}

	/*
	 * Otherwise, run only the specified tests.
	 */
	printf("1..%d\n", argc - 1);
	for (int i = 1; i < argc; i++)
	{
		int found = 0;
		for (int j = 0; j < test_count; j++) {
			if (strncmp(argv[i], all_tests[j].t_name,
			    strlen(argv[i])) == 0) {
				found = 1;
				execute(i, all_tests + j);
				break;
			}
		}

		if (found == 0)
			errx(-1, "No such test '%s'", argv[i]);
	}

	return (0);
}

int
execute(int id, struct test *t) {
	int result;

	pid_t pid = fork();
	if (pid < 0)
		err(-1, "fork");
	if (pid) {
		/* Parent: wait for result from child. */
		int status;
		while (waitpid(pid, &status, 0) != pid) {}
		if (WIFEXITED(status))
			result = WEXITSTATUS(status);
		else
			result = FAILED;
	} else {
		/* Child process: run the test. */
		exit(t->t_run());
	}

	printf("%s %d - %s\n",
		(result == PASSED) ? "ok" : "not ok",
		id, t->t_name);

	return (result);
}