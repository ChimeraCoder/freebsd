
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

struct stub {
    char *name;
    int (*f)();
};

extern char *__progname;
extern struct stub entry_points[];

int
main(int argc, char **argv, char **envp)
{
    char *slash, *basename;
    struct stub *ep;

    if(argv[0] == NULL || *argv[0] == '\0')
	crunched_usage();

    slash = strrchr(argv[0], '/');
    basename = slash? slash+1 : argv[0];

    for(ep=entry_points; ep->name != NULL; ep++)
	if(!strcmp(basename, ep->name)) break;

    if(ep->name)
	return ep->f(argc, argv, envp);
    else {
	fprintf(stderr, "%s: %s not compiled in\n", EXECNAME, basename);
	crunched_usage();
    }
}


int
crunched_here(char *path)
{
    char *slash, *basename;
    struct stub *ep;

    slash = strrchr(path, '/');
    basename = slash? slash+1 : path;

    for(ep=entry_points; ep->name != NULL; ep++)
	if(!strcmp(basename, ep->name))
	    return 1;
    return 0;
}


int
crunched_main(int argc, char **argv, char **envp)
{
    char *slash;
    struct stub *ep;
    int columns, len;

    if(argc <= 1)
	crunched_usage();

    slash = strrchr(argv[1], '/');
    __progname = slash? slash+1 : argv[1];

    return main(--argc, ++argv, envp);
}


int
crunched_usage()
{
    int columns, len;
    struct stub *ep;

    fprintf(stderr, "usage: %s <prog> <args> ..., where <prog> is one of:\n",
	    EXECNAME);
    columns = 0;
    for(ep=entry_points; ep->name != NULL; ep++) {
	len = strlen(ep->name) + 1;
	if(columns+len < 80)
	    columns += len;
	else {
	    fprintf(stderr, "\n");
	    columns = len;
	}
	fprintf(stderr, " %s", ep->name);
    }
    fprintf(stderr, "\n");
    exit(1);
}

/* end of crunched_main.c */