
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <security/pam_appl.h>

#include "openpam_impl.h"

#define MIN_LINE_LENGTH 128

/*
 * OpenPAM extension
 *
 * Read a line from a file.
 */

char *
openpam_readline(FILE *f, int *lineno, size_t *lenp)
{
	char *line;
	size_t len, size;
	int ch;

	line = NULL;
	if (openpam_straddch(&line, &size, &len, 0) != 0)
		return (NULL);
	for (;;) {
		ch = fgetc(f);
		/* strip comment */
		if (ch == '#') {
			do {
				ch = fgetc(f);
			} while (ch != EOF && ch != '\n');
		}
		/* eof */
		if (ch == EOF) {
			/* done */
			break;
		}
		/* eol */
		if (ch == '\n') {
			if (lineno != NULL)
				++*lineno;
			/* skip blank lines */
			if (len == 0)
				continue;
			/* continuation */
			if (line[len - 1] == '\\') {
				line[--len] = '\0';
				continue;
			}
			/* done */
			break;
		}
		/* anything else */
		if (openpam_straddch(&line, &size, &len, ch) != 0)
			goto fail;
	}
	if (len == 0)
		goto fail;
	if (lenp != NULL)
		*lenp = len;
	openpam_log(PAM_LOG_LIBDEBUG, "returning '%s'", line);
	return (line);
fail:
	FREE(line);
	return (NULL);
}

/**
 * DEPRECATED openpam_readlinev
 *
 * The =openpam_readline function reads a line from a file, and returns it
 * in a NUL-terminated buffer allocated with =!malloc.
 *
 * The =openpam_readline function performs a certain amount of processing
 * on the data it reads:
 *
 *  - Comments (introduced by a hash sign) are stripped.
 *
 *  - Blank lines are ignored.
 *
 *  - If a line ends in a backslash, the backslash is stripped and the
 *    next line is appended.
 *
 * If =lineno is not =NULL, the integer variable it points to is
 * incremented every time a newline character is read.
 *
 * If =lenp is not =NULL, the length of the line (not including the
 * terminating NUL character) is stored in the variable it points to.
 *
 * The caller is responsible for releasing the returned buffer by passing
 * it to =!free.
 *
 * >openpam_readlinev
 * >openpam_readword
 */