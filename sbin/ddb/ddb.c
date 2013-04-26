
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

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "ddb.h"

void ddb_readfile(char *file);
void ddb_main(int argc, char *argv[]);

void
usage(void)
{

	fprintf(stderr, "usage: ddb capture [-M core] [-N system] print\n");
	fprintf(stderr, "       ddb capture [-M core] [-N system] status\n");
	fprintf(stderr, "       ddb script scriptname\n");
	fprintf(stderr, "       ddb script scriptname=script\n");
	fprintf(stderr, "       ddb scripts\n");
	fprintf(stderr, "       ddb unscript scriptname\n");
	fprintf(stderr, "       ddb pathname\n");
	exit(EX_USAGE);
}

void
ddb_readfile(char *filename)
{
	char    buf[BUFSIZ];
	FILE*	f;

	if ((f = fopen(filename, "r")) == NULL)
		err(EX_UNAVAILABLE, "fopen: %s", filename);

#define WHITESP		" \t"
#define MAXARG	 	2
	while (fgets(buf, BUFSIZ, f)) {
		int argc = 0;
		char *argv[MAXARG];
		size_t spn;

		spn = strlen(buf);
		if (buf[spn-1] == '\n')
			buf[spn-1] = '\0';

		spn = strspn(buf, WHITESP);
		argv[0] = buf + spn;
		if (*argv[0] == '#' || *argv[0] == '\0')
			continue;
		argc++;

		spn = strcspn(argv[0], WHITESP);
		argv[1] = argv[0] + spn + strspn(argv[0] + spn, WHITESP);
		argv[0][spn] = '\0';
		if (*argv[1] != '\0')
			argc++;

#ifdef DEBUG
		{
			int i;
			printf("argc = %d\n", argc);
			for (i = 0; i < argc; i++) {
				printf("arg[%d] = %s\n", i, argv[i]);
			}
		}
#endif
		ddb_main(argc, argv);
	}
	fclose(f);
}

void
ddb_main(int argc, char *argv[])
{

	if (argc < 1)
		usage();

	if (strcmp(argv[0], "capture") == 0)
		ddb_capture(argc, argv);
	else if (strcmp(argv[0], "script") == 0)
		ddb_script(argc, argv);
	else if (strcmp(argv[0], "scripts") == 0)
		ddb_scripts(argc, argv);
	else if (strcmp(argv[0], "unscript") == 0)
		ddb_unscript(argc, argv);
	else
		usage();
}

int
main(int argc, char *argv[])
{

	/*
	 * If we've only got one argument and it's an absolute path to a file,
	 * interpret as a file to be read in.
	 */
	if (argc == 2 && argv[1][0] == '/' && access(argv[1], R_OK) == 0)
		ddb_readfile(argv[1]);
	else
		ddb_main(argc-1, argv+1);
	exit(EX_OK);
}