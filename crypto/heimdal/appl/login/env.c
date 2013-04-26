
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

#include "login_locl.h"
RCSID("$Id$");

/*
 * the environment we will send to execle and the shell.
 */

char **env;
int num_env;

void
extend_env(char *str)
{
    env = realloc(env, (num_env + 1) * sizeof(*env));
    if(env == NULL)
	errx(1, "Out of memory!");
    env[num_env++] = str;
}

void
add_env(const char *var, const char *value)
{
    int i;
    char *str;
    asprintf(&str, "%s=%s", var, value);
    if(str == NULL)
	errx(1, "Out of memory!");
    for(i = 0; i < num_env; i++)
	if(strncmp(env[i], var, strlen(var)) == 0 &&
	   env[i][strlen(var)] == '='){
	    free(env[i]);
	    env[i] = str;
	    return;
	}

    extend_env(str);
}

#if !HAVE_DECL_ENVIRON
extern char **environ;
#endif


void
copy_env(void)
{
    char **p;
    for(p = environ; *p; p++)
	extend_env(*p);
}

void
login_read_env(const char *file)
{
    char **newenv;
    char *p;
    int i, j;

    newenv = NULL;
    i = read_environment(file, &newenv);
    for (j = 0; j < i; j++) {
	p = strchr(newenv[j], '=');
	if (p == NULL)
	    errx(1, "%s: missing = in string %s",
		 file, newenv[j]);
	*p++ = 0;
	add_env(newenv[j], p);
	*--p = '=';
	free(newenv[j]);
    }
    free(newenv);
}