
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
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");


const char value1[] = "Large ------------------ value";
const char value2[] = "Small -- value";
char nameValuePair[] = "less=more";
const char name[] = "PATH";
const char name2[] = "SHELL";
const int MaxIterations = 1000000;
const char Tabs[] = "\t\t\t";


static int
report_time(const char *action, struct timeval *startTime,
    struct timeval *endTime)
{
	int actionLen;
	int numTabs;

	actionLen = strlen(action);
	numTabs = 3 - actionLen / 8;

	return (printf("Time spent executing %s:%.*s%f\n", action, numTabs, Tabs,
	    (endTime->tv_sec - startTime->tv_sec) +
	    (double)(endTime->tv_usec - startTime->tv_usec) / 1000000));
}


int
main(int argc, char **argv)
{
	int iterations;
	struct rusage endUsage;
	struct rusage startUsage;

	/*
	 * getenv() on the existing environment.
	 */
	getrusage(RUSAGE_SELF, &startUsage);

	/* Iterate over setting variable. */
	for (iterations = 0; iterations < MaxIterations; iterations++)
		if (getenv(name) == NULL)
			err(EXIT_FAILURE, "getenv(name)");

	getrusage(RUSAGE_SELF, &endUsage);

	report_time("getenv(name)", &startUsage.ru_utime, &endUsage.ru_utime);


	/*
	 * setenv() a variable with a large value.
	 */
	getrusage(RUSAGE_SELF, &startUsage);

	/* Iterate over setting variable. */
	for (iterations = 0; iterations < MaxIterations; iterations++)
		if (setenv(name, value1, 1) == -1)
			err(EXIT_FAILURE, "setenv(name, value1, 1)");

	getrusage(RUSAGE_SELF, &endUsage);

	report_time("setenv(name, value1, 1)", &startUsage.ru_utime,
	    &endUsage.ru_utime);


	/*
	 * getenv() the new variable on the new environment.
	 */
	getrusage(RUSAGE_SELF, &startUsage);

	/* Iterate over setting variable. */
	for (iterations = 0; iterations < MaxIterations; iterations++)
		/* Set large value to variable. */
		if (getenv(name) == NULL)
			err(EXIT_FAILURE, "getenv(name)");

	getrusage(RUSAGE_SELF, &endUsage);

	report_time("getenv(name)", &startUsage.ru_utime, &endUsage.ru_utime);


	/*
	 * getenv() a different variable on the new environment.
	 */
	getrusage(RUSAGE_SELF, &startUsage);

	/* Iterate over setting variable. */
	for (iterations = 0; iterations < MaxIterations; iterations++)
		/* Set large value to variable. */
		if (getenv(name2) == NULL)
			err(EXIT_FAILURE, "getenv(name2)");

	getrusage(RUSAGE_SELF, &endUsage);

	report_time("getenv(name2)", &startUsage.ru_utime, &endUsage.ru_utime);


	/*
	 * setenv() a variable with a small value.
	 */
	getrusage(RUSAGE_SELF, &startUsage);

	/* Iterate over setting variable. */
	for (iterations = 0; iterations < MaxIterations; iterations++)
		if (setenv(name, value2, 1) == -1)
			err(EXIT_FAILURE, "setenv(name, value2, 1)");

	getrusage(RUSAGE_SELF, &endUsage);

	report_time("setenv(name, value2, 1)", &startUsage.ru_utime,
	    &endUsage.ru_utime);


	/*
	 * getenv() a different variable on the new environment.
	 */
	getrusage(RUSAGE_SELF, &startUsage);

	/* Iterate over setting variable. */
	for (iterations = 0; iterations < MaxIterations; iterations++)
		/* Set large value to variable. */
		if (getenv(name2) == NULL)
			err(EXIT_FAILURE, "getenv(name)");

	getrusage(RUSAGE_SELF, &endUsage);

	report_time("getenv(name)", &startUsage.ru_utime, &endUsage.ru_utime);


	/*
	 * getenv() a different variable on the new environment.
	 */
	getrusage(RUSAGE_SELF, &startUsage);

	/* Iterate over setting variable. */
	for (iterations = 0; iterations < MaxIterations; iterations++)
		/* Set large value to variable. */
		if (getenv(name2) == NULL)
			err(EXIT_FAILURE, "getenv(name2)");

	getrusage(RUSAGE_SELF, &endUsage);

	report_time("getenv(name2)", &startUsage.ru_utime, &endUsage.ru_utime);


	/*
	 * putenv() a variable with a small value.
	 */
	getrusage(RUSAGE_SELF, &startUsage);

	/* Iterate over setting variable. */
	for (iterations = 0; iterations < MaxIterations; iterations++)
		if (putenv(nameValuePair) == -1)
			err(EXIT_FAILURE, "putenv(nameValuePair)");

	getrusage(RUSAGE_SELF, &endUsage);

	report_time("putenv(nameValuePair)", &startUsage.ru_utime,
	    &endUsage.ru_utime);


	exit(EXIT_SUCCESS);
}