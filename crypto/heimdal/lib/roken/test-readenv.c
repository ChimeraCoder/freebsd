
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

#include "roken.h"
#include "test-mem.h"

char *s1 = "VAR1=VAL1#comment\n\
VAR2=VAL2 VAL2 #comment\n\
#this another comment\n\
\n\
VAR3=FOO";

char *s2 = "VAR1=ENV2\n\
";

static void
make_file(char *tmpl, size_t l)
{
    int fd;
    strlcpy(tmpl, "env.XXXXXX", l);
    fd = mkstemp(tmpl);
    if(fd < 0)
	err(1, "mkstemp");
    close(fd);
}

static void
write_file(const char *fn, const char *s)
{
    FILE *f;
    f = fopen(fn, "w");
    if(f == NULL) {
	unlink(fn);
	err(1, "fopen");
    }
    if(fwrite(s, 1, strlen(s), f) != strlen(s))
	err(1, "short write");
    if(fclose(f) != 0) {
	unlink(fn);
	err(1, "fclose");
    }
}

int
main(int argc, char **argv)
{
    char **env = NULL;
    int count = 0;
    char fn[MAXPATHLEN];
    int error = 0;

    make_file(fn, sizeof(fn));

    write_file(fn, s1);
    count = read_environment(fn, &env);
    if(count != 3) {
	warnx("test 1: variable count %d != 3", count);
	error++;
    }

    write_file(fn, s2);
    count = read_environment(fn, &env);
    if(count != 1) {
	warnx("test 2: variable count %d != 1", count);
	error++;
    }

    unlink(fn);
    count = read_environment(fn, &env);
    if(count != 0) {
	warnx("test 3: variable count %d != 0", count);
	error++;
    }
    for(count = 0; env && env[count]; count++);
    if(count != 3) {
	warnx("total variable count %d != 3", count);
	error++;
    }
    free_environment(env);


    return error;
}