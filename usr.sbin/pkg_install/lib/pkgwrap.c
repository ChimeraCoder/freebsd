
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

#include "lib.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#undef main

#define SEPARATORS " \t"

extern char **environ;

int
main(int argc, char **argv)
{
    FILE *f;
    char buffer[FILENAME_MAX], *cp, *verstr;
    int len;

    if (getenv("PKG_NOWRAP") != NULL)
	goto nowrap;
    f = fopen(PKG_WRAPCONF_FNAME, "r");
    if (f == NULL)
	goto nowrap;
    cp = fgets(buffer, 256, f);
    fclose(f);
    if (cp == NULL)
	goto nowrap;
    len = strlen(cp);
    if (cp[len - 1] == '\n')
	cp[len - 1] = '\0';
    while (strchr(SEPARATORS, *cp) != NULL)
	cp++;
    verstr = cp;
    cp = strpbrk(cp, SEPARATORS);
    if (cp == NULL)
	goto nowrap;
    *cp = '\0';
    for (cp = verstr; *cp != '\0'; cp++)
	if (isdigit(*cp) == 0)
	    goto nowrap;
    if (atoi(verstr) < PKG_INSTALL_VERSION)
	goto nowrap;
    cp++;
    while (*cp != '\0' && strchr(SEPARATORS, *cp) != NULL)
	cp++;
    if (*cp == '\0')
	goto nowrap;
    bcopy(cp, buffer, strlen(cp) + 1);
    cp = strpbrk(buffer, SEPARATORS);
    if (cp != NULL)
	*cp = '\0';
    if (!isdir(buffer))
	goto nowrap;
    cp = strrchr(argv[0], '/');
    if (cp == NULL)
	cp = argv[0];
    else
	cp++;
    strlcat(buffer, "/", sizeof(buffer));
    strlcat(buffer, cp, sizeof(buffer));
    setenv("PKG_NOWRAP", "1", 1);
    execve(buffer, argv, environ);

nowrap:
    unsetenv("PKG_NOWRAP");
    return(real_main(argc, argv));
}