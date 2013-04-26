
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

/* System Headers */

#include <sys/types.h>
#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Local headers */

#include "at.h"
#include "perm.h"
#include "privs.h"

/* Macros */

#define MAXUSERID 10

/* Structures and unions */

/* Function declarations */

static int check_for_user(FILE *fp,const char *name);

/* Local functions */

static int check_for_user(FILE *fp,const char *name)
{
    char *buffer;
    size_t len;
    int found = 0;

    len = strlen(name);
    if ((buffer = malloc(len+2)) == NULL)
	errx(EXIT_FAILURE, "virtual memory exhausted");

    while(fgets(buffer, len+2, fp) != NULL)
    {
	if ((strncmp(name, buffer, len) == 0) &&
	    (buffer[len] == '\n'))
	{
	    found = 1;
	    break;
	}
    }
    fclose(fp);
    free(buffer);
    return found;
}
/* Global functions */
int check_permission(void)
{
    FILE *fp;
    uid_t uid = geteuid();
    struct passwd *pentry;

    if (uid==0)
	return 1;

    if ((pentry = getpwuid(uid)) == NULL)
	err(EXIT_FAILURE, "cannot access user database");

    PRIV_START

    fp=fopen(PERM_PATH "at.allow","r");

    PRIV_END

    if (fp != NULL)
    {
	return check_for_user(fp, pentry->pw_name);
    }
    else if (errno == ENOENT)
    {

	PRIV_START

	fp=fopen(PERM_PATH "at.deny", "r");

	PRIV_END

	if (fp != NULL)
	{
	    return !check_for_user(fp, pentry->pw_name);
	}
	else if (errno != ENOENT)
	    warn("at.deny");
    }
    else
	warn("at.allow");
    return 0;
}