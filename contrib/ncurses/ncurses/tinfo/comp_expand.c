
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

/****************************************************************************
 *  Author: Thomas E. Dickey <dickey@clark.net> 1998                        *
 ****************************************************************************/

#include <curses.priv.h>

#include <ctype.h>
#include <tic.h>

MODULE_ID("$Id: comp_expand.c,v 1.20 2008/08/16 19:29:42 tom Exp $")

static int
trailing_spaces(const char *src)
{
    while (*src == ' ')
	src++;
    return *src == 0;
}

/* this deals with differences over whether 0x7f and 0x80..0x9f are controls */
#define REALCTL(s) (UChar(*(s)) < 127 && iscntrl(UChar(*(s))))
#define REALPRINT(s) (UChar(*(s)) < 127 && isprint(UChar(*(s))))

NCURSES_EXPORT(char *)
_nc_tic_expand(const char *srcp, bool tic_format, int numbers)
{
    static char *buffer;
    static size_t length;

    int bufp;
    const char *str = VALID_STRING(srcp) ? srcp : "\0\0";
    bool islong = (strlen(str) > 3);
    size_t need = (2 + strlen(str)) * 4;
    int ch;

#if NO_LEAKS
    if (srcp == 0) {
	if (buffer != 0) {
	    FreeAndNull(buffer);
	    length = 0;
	}
	return 0;
    }
#endif
    if (buffer == 0 || need > length) {
	if ((buffer = typeRealloc(char, length = need, buffer)) == 0)
	      return 0;
    }

    bufp = 0;
    while ((ch = UChar(*str)) != 0) {
	if (ch == '%' && REALPRINT(str + 1)) {
	    buffer[bufp++] = *str++;
	    /*
	     * Though the character literals are more compact, most
	     * terminal descriptions use numbers and are not easy
	     * to read in character-literal form.
	     */
	    switch (numbers) {
	    case -1:
		if (str[0] == S_QUOTE
		    && str[1] != '\\'
		    && REALPRINT(str + 1)
		    && str[2] == S_QUOTE) {
		    sprintf(buffer + bufp, "{%d}", str[1]);
		    bufp += strlen(buffer + bufp);
		    str += 2;
		} else {
		    buffer[bufp++] = *str;
		}
		break;
		/*
		 * If we have a "%{number}", try to translate it into
		 * a "%'char'" form, since that will run a little faster
		 * when we're interpreting it.  Also, having one form
		 * for the constant makes it simpler to compare terminal
		 * descriptions.
		 */
	    case 1:
		if (str[0] == L_BRACE
		    && isdigit(UChar(str[1]))) {
		    char *dst = 0;
		    long value = strtol(str + 1, &dst, 0);
		    if (dst != 0
			&& *dst == R_BRACE
			&& value < 127
			&& value != '\\'	/* FIXME */
			&& isprint((int) value)) {
			ch = (int) value;
			buffer[bufp++] = S_QUOTE;
			if (ch == '\\'
			    || ch == S_QUOTE)
			    buffer[bufp++] = '\\';
			buffer[bufp++] = (char) ch;
			buffer[bufp++] = S_QUOTE;
			str = dst;
		    } else {
			buffer[bufp++] = *str;
		    }
		} else {
		    buffer[bufp++] = *str;
		}
		break;
	    default:
		buffer[bufp++] = *str;
		break;
	    }
	} else if (ch == 128) {
	    buffer[bufp++] = '\\';
	    buffer[bufp++] = '0';
	} else if (ch == '\033') {
	    buffer[bufp++] = '\\';
	    buffer[bufp++] = 'E';
	} else if (ch == '\\' && tic_format && (str == srcp || str[-1] != '^')) {
	    buffer[bufp++] = '\\';
	    buffer[bufp++] = '\\';
	} else if (ch == ' ' && tic_format && (str == srcp ||
					       trailing_spaces(str))) {
	    buffer[bufp++] = '\\';
	    buffer[bufp++] = 's';
	} else if ((ch == ',' || ch == ':' || ch == '^') && tic_format) {
	    buffer[bufp++] = '\\';
	    buffer[bufp++] = (char) ch;
	} else if (REALPRINT(str)
		   && (ch != ','
		       && ch != ':'
		       && !(ch == '!' && !tic_format)
		       && ch != '^'))
	    buffer[bufp++] = (char) ch;
#if 0				/* FIXME: this would be more readable (in fact the whole 'islong' logic should be removed) */
	else if (ch == '\b') {
	    buffer[bufp++] = '\\';
	    buffer[bufp++] = 'b';
	} else if (ch == '\f') {
	    buffer[bufp++] = '\\';
	    buffer[bufp++] = 'f';
	} else if (ch == '\t' && islong) {
	    buffer[bufp++] = '\\';
	    buffer[bufp++] = 't';
	}
#endif
	else if (ch == '\r' && (islong || (strlen(srcp) > 2 && str[1] == '\0'))) {
	    buffer[bufp++] = '\\';
	    buffer[bufp++] = 'r';
	} else if (ch == '\n' && islong) {
	    buffer[bufp++] = '\\';
	    buffer[bufp++] = 'n';
	}
#define UnCtl(c) ((c) + '@')
	else if (REALCTL(str) && ch != '\\'
		 && (!islong || isdigit(UChar(str[1])))) {
	    (void) sprintf(&buffer[bufp], "^%c", UnCtl(ch));
	    bufp += 2;
	} else {
	    (void) sprintf(&buffer[bufp], "\\%03o", ch);
	    bufp += 4;
	}

	str++;
    }

    buffer[bufp] = '\0';
    return (buffer);
}