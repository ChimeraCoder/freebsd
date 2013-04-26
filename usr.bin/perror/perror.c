
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <locale.h>
#include <sys/errno.h>

static void usage(void);

int 
main(int argc, char **argv)
{
	char *cp;
	char *errstr;
	long errnum;

	(void) setlocale(LC_MESSAGES, "");
	if (argc != 2)
		usage();

	errno = 0;

	errnum = strtol(argv[1], &cp, 0);

	if (errno != 0)
		err(1, NULL);

	if ((errstr = strerror(errnum)) == NULL) 
		err(1, NULL);

	printf("%s\n", errstr);

	exit(0);
}

static void 
usage(void)
{
	fprintf(stderr, "usage: perror number\n");
	exit(1);
}