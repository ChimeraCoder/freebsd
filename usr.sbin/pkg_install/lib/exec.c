
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
#include <err.h>

/*
 * Unusual system() substitute.  Accepts format string and args,
 * builds and executes command.  Returns exit code.
 */

int
vsystem(const char *fmt, ...)
{
    va_list args;
    char *cmd;
    int ret, maxargs;

    maxargs = sysconf(_SC_ARG_MAX);
    maxargs -= 32;			/* some slop for the sh -c */
    cmd = malloc(maxargs);
    if (!cmd) {
	warnx("vsystem can't alloc arg space");
	return 1;
    }

    va_start(args, fmt);
    if (vsnprintf(cmd, maxargs, fmt, args) > maxargs) {
	warnx("vsystem args are too long");
	va_end(args);
	return 1;
    }
#ifdef DEBUG
printf("Executing %s\n", cmd);
#endif
    ret = system(cmd);
    va_end(args);
    free(cmd);
    return ret;
}

char *
vpipe(const char *fmt, ...)
{
   FILE *fp;
   char *cmd, *rp;
   int maxargs;
   va_list args;

    rp = malloc(MAXPATHLEN);
    if (!rp) {
	warnx("vpipe can't alloc buffer space");
	return NULL;
    }
    maxargs = sysconf(_SC_ARG_MAX);
    maxargs -= 32;			    /* some slop for the sh -c */
    cmd = alloca(maxargs);
    if (!cmd) {
	warnx("vpipe can't alloc arg space");
	return NULL;
    }

    va_start(args, fmt);
    if (vsnprintf(cmd, maxargs, fmt, args) > maxargs) {
	warnx("vsystem args are too long");
	va_end(args);
	return NULL;
    }
#ifdef DEBUG
    fprintf(stderr, "Executing %s\n", cmd);
#endif
    fflush(NULL);
    fp = popen(cmd, "r");
    if (fp == NULL) {
	warnx("popen() failed");
	va_end(args);
	return NULL;
    }
    get_string(rp, MAXPATHLEN, fp);
#ifdef DEBUG
    fprintf(stderr, "Returned %s\n", rp);
#endif
    va_end(args);
    if (pclose(fp) || (strlen(rp) == 0)) {
	free(rp);
	return NULL;
    }
    return rp;
}