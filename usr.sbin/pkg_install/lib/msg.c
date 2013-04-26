
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
#include <paths.h>

/* Die a relatively simple death */
void
upchuck(const char *message)
{
    cleanup(0);
    errx(1, "fatal error during execution: %s", message);
}

/*
 * As a yes/no question, prompting from the varargs string and using
 * default if user just hits return.
 */
Boolean
y_or_n(Boolean def, const char *msg, ...)
{
    va_list args;
    int ch = 0;
    FILE *tty;

    va_start(args, msg);
    /*
     * Need to open /dev/tty because file collection may have been
     * collected on stdin
     */
    tty = fopen(_PATH_TTY, "r");
    if (!tty) {
	cleanup(0);
	errx(2, "can't open %s!", _PATH_TTY);
    }
    while (ch != 'Y' && ch != 'N') {
	vfprintf(stderr, msg, args);
	if (def)
	    fprintf(stderr, " [yes]? ");
	else
	    fprintf(stderr, " [no]? ");
	fflush(stderr);
	if (AutoAnswer) {
	    ch = (AutoAnswer == YES) ? 'Y' : 'N';
	    fprintf(stderr, "%c\n", ch);
	}
	else
	    ch = toupper(fgetc(tty));
	if (ch == '\n')
	    ch = (def) ? 'Y' : 'N';
    }
    fclose(tty) ;
    va_end(args);
    return (ch == 'Y') ? TRUE : FALSE;
}