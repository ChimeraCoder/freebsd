
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

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *
stream_name(FILE *s)
{

	if (s == stdin)
		return "stdin";
	if (s == stdout)
		return "stdout";
	if (s == stderr)
		return "stderr";
	/* This should not happen. */
	abort();
}

static void
change_buf(FILE *s, const char *bufmode)
{
	char *unit;
	size_t bufsize;
	int mode;

	bufsize = 0;
	if (bufmode[0] == '0' && bufmode[1] == '\0')
		mode = _IONBF;
	else if (bufmode[0] == 'L' && bufmode[1] == '\0')
		mode = _IOLBF;
	else if (bufmode[0] == 'B' && bufmode[1] == '\0') {
		mode = _IOFBF;
		bufsize = 0;
	} else {
		/*
		 * This library being preloaded, depending on libutil
		 * would lead to excessive namespace pollution.
		 * Thus we do not use expand_number().
		 */
		errno = 0;
		bufsize = strtol(bufmode, &unit, 0);
		if (errno == EINVAL || errno == ERANGE || unit == bufmode)
			warn("Wrong buffer mode '%s' for %s", bufmode,
			    stream_name(s));
		switch (*unit) {
		case 'G':
			bufsize *= 1024 * 1024 * 1024;
			break;
		case 'M':
			bufsize *= 1024 * 1024;
			break;
		case 'k':
			bufsize *= 1024;
			break;
		case '\0':
			break;
		default:
			warnx("Unknown suffix '%c' for %s", *unit,
			    stream_name(s));
			return;
		}
		mode = _IOFBF;
	}
	if (setvbuf(s, NULL, mode, bufsize) != 0)
		warn("Cannot set buffer mode '%s' for %s", bufmode,
		    stream_name(s));
}

__attribute__ ((constructor)) static void
stdbuf(void)
{
	char *i_mode, *o_mode, *e_mode;

	i_mode = getenv("_STDBUF_I");
	o_mode = getenv("_STDBUF_O");
	e_mode = getenv("_STDBUF_E");

	if (e_mode != NULL)
		change_buf(stderr, e_mode);
	if (i_mode != NULL)
		change_buf(stdin, i_mode);
	if (o_mode != NULL)
		change_buf(stdout, o_mode);
}