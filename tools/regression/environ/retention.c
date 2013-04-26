
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");


extern char **environ;
const char *envName = "FOOBAR";
const char *envValSmall = "Hi";
const char *envValLarge = "Hi, again";
const char *envValAny = "Any value";


int
main(int argc, char **argv)
{
	const char *env1 = NULL;
	const char *env2 = NULL;
	const char *env3 = NULL;
	const char *env4 = NULL;
	const char *env5 = NULL;
	int testNdx;

	/* Clean slate. */
	environ = NULL;
	testNdx = 0;

	/* Initial value of variable. */
	if (getenv(envName) != NULL)
		printf("not ");
	printf("ok %d - getenv(\"%s\")\n", ++testNdx, envName);

	/* Set value of variable to smaller value and get value. */
	if ((setenv(envName, envValSmall, 1) != 0) ||
	    ((env1 = getenv(envName)) == NULL) ||
	    (strcmp(env1, envValSmall) != 0))
		printf("not ");
	printf("ok %d - setenv(\"%s\", \"%s\", 1)\n", ++testNdx, envName,
	    envValSmall);

	/* Unset variable. */
	if ((unsetenv(envName) == -1) || ((env2 = getenv(envName)) != NULL))
		printf("not ");
	printf("ok %d - unsetenv(\"%s\")\n", ++testNdx, envName);

	/* Set variable to bigger value and get value. */
	if ((setenv(envName, envValLarge, 1) != 0) ||
	    ((env3 = getenv(envName)) == NULL) ||
	    (strcmp(env3, envValLarge) != 0))
		printf("not ");
	printf("ok %d - setenv(\"%s\", \"%s\", 1)\n", ++testNdx, envName,
	    envValLarge);

	/* Set variable to smaller value and get value. */
	if ((setenv(envName, envValSmall, 1) != 0) ||
	    ((env4 = getenv(envName)) == NULL) ||
	    (strcmp(env4, envValSmall) != 0))
		printf("not ");
	printf("ok %d - setenv(\"%s\", \"%s\", 1)\n", ++testNdx, envName,
	    envValSmall);

	/* Set variable to any value without overwrite and get value. */
	if ((setenv(envName, envValAny, 0) != 0) ||
	    ((env5 = getenv(envName)) == NULL) ||
	    (strcmp(env5, envValAny) == 0))
		printf("not ");
	printf("ok %d - setenv(\"%s\", \"%s\", 0)\n", ++testNdx, envName,
	    envValAny);

	/*
	 * Verify FreeBSD-ism about allowing a program to keep old pointers without
	 * risk of segfaulting.
	 */
	if ((strcmp(env1, envValSmall) != 0) ||
	    (strcmp(env3, envValSmall) != 0) ||
	    (strcmp(env4, envValSmall) != 0))
		printf("not ");
	printf("ok %d - old variables point to valid memory\n", ++testNdx);

	exit(EXIT_SUCCESS);
}