
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


#include <config.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "roken.h"

/* find assignment in env list; len is length of variable including
 * equal
 */

static int
find_var(char **env, char *assignment, size_t len)
{
    int i;
    for(i = 0; env != NULL && env[i] != NULL; i++)
	if(strncmp(env[i], assignment, len) == 0)
	    return i;
    return -1;
}

/*
 * return count of environment assignments from open file F in
 * assigned and list of malloced strings in env, return 0 or errno
 * number
 */

static int
read_env_file(FILE *F, char ***env, int *assigned)
{
    int idx = 0;
    int i;
    char **l;
    char buf[BUFSIZ], *p, *r;
    char **tmp;
    int ret = 0;

    *assigned = 0;

    for(idx = 0; *env != NULL && (*env)[idx] != NULL; idx++);
    l = *env;

    /* This is somewhat more relaxed on what it accepts then
     * Wietses sysv_environ from K4 was...
     */
    while (fgets(buf, BUFSIZ, F) != NULL) {
	buf[strcspn(buf, "#\n")] = '\0';

	for(p = buf; isspace((unsigned char)*p); p++);
	if (*p == '\0')
	    continue;

	/* Here one should check that it's a 'valid' env string... */
	r = strchr(p, '=');
	if (r == NULL)
	    continue;

	if((i = find_var(l, p, r - p + 1)) >= 0) {
	    char *val = strdup(p);
	    if(val == NULL) {
		ret = ENOMEM;
		break;
	    }
	    free(l[i]);
	    l[i] = val;
	    (*assigned)++;
	    continue;
	}

	tmp = realloc(l, (idx+2) * sizeof (char *));
	if(tmp == NULL) {
	    ret = ENOMEM;
	    break;
	}

	l = tmp;
	l[idx] = strdup(p);
	if(l[idx] == NULL) {
	    ret = ENOMEM;
	    break;
	}
	l[++idx] = NULL;
	(*assigned)++;
    }
    if(ferror(F))
	ret = errno;
    *env = l;
    return ret;
}

/*
 * return count of environment assignments from file and
 * list of malloced strings in `env'
 */

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
read_environment(const char *file, char ***env)
{
    int assigned;
    FILE *F;

    if ((F = fopen(file, "r")) == NULL)
	return 0;

    read_env_file(F, env, &assigned);
    fclose(F);
    return assigned;
}

ROKEN_LIB_FUNCTION void ROKEN_LIB_CALL
free_environment(char **env)
{
    int i;
    if (env == NULL)
	return;
    for (i = 0; env[i]; i++)
	free(env[i]);
    free(env);
}