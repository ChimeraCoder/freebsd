
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
 *  Author: Thomas E. Dickey 2001                                           *
 ****************************************************************************/

#include <curses.priv.h>

#include <ctype.h>

MODULE_ID("$Id: varargs.c,v 1.7 2008/08/03 15:42:49 tom Exp $")

#ifdef TRACE

#define MAX_PARMS 10

typedef enum {
    atUnknown = 0, atInteger, atFloat, atPoint, atString
} ARGTYPE;

#define VA_INT(type) ival = va_arg(ap, type)
#define VA_FLT(type) fval = va_arg(ap, type)
#define VA_PTR(type) pval = (char *)va_arg(ap, type)
#define VA_STR(type) sval = va_arg(ap, type)

#define MyBuffer _nc_globals.tracearg_buf
#define MyLength _nc_globals.tracearg_used

/*
 * Returns a string that represents the parameter list of a printf-style call.
 */
NCURSES_EXPORT(char *)
_nc_varargs(const char *fmt, va_list ap)
{
    static char dummy[] = "";

    char buffer[BUFSIZ];
    const char *param;
    int n;

    if (fmt == 0 || *fmt == '\0')
	return dummy;
    if (MyLength == 0)
	MyBuffer = typeMalloc(char, MyLength = BUFSIZ);
    if (MyBuffer == 0)
	return dummy;
    *MyBuffer = '\0';

    while (*fmt != '\0') {
	if (*fmt == '%') {
	    char *pval = 0;	/* avoid const-cast */
	    const char *sval = "";
	    double fval = 0.0;
	    int done = FALSE;
	    int ival = 0;
	    int type = 0;
	    ARGTYPE parm[MAX_PARMS];
	    int parms = 0;
	    ARGTYPE used = atUnknown;

	    while (*++fmt != '\0' && !done) {

		if (*fmt == '*') {
		    VA_INT(int);
		    if (parms < MAX_PARMS)
			parm[parms++] = atInteger;
		} else if (isalpha(UChar(*fmt))) {
		    done = TRUE;
		    switch (*fmt) {
		    case 'Z':	/* FALLTHRU */
		    case 'h':	/* FALLTHRU */
		    case 'l':	/* FALLTHRU */
			done = FALSE;
			type = *fmt;
			break;
		    case 'i':	/* FALLTHRU */
		    case 'd':	/* FALLTHRU */
		    case 'u':	/* FALLTHRU */
		    case 'x':	/* FALLTHRU */
		    case 'X':	/* FALLTHRU */
			if (type == 'l')
			    VA_INT(long);
			else if (type == 'Z')
			    VA_INT(size_t);
			else
			    VA_INT(int);
			used = atInteger;
			break;
		    case 'f':	/* FALLTHRU */
		    case 'e':	/* FALLTHRU */
		    case 'E':	/* FALLTHRU */
		    case 'g':	/* FALLTHRU */
		    case 'G':	/* FALLTHRU */
			VA_FLT(double);
			used = atFloat;
			break;
		    case 'c':
			VA_INT(int);
			used = atInteger;
			break;
		    case 's':
			VA_STR(const char *);
			used = atString;
			break;
		    case 'p':
			VA_PTR(void *);
			used = atPoint;
			break;
		    case 'n':
			VA_PTR(int *);
			used = atPoint;
			break;
		    default:
			break;
		    }
		} else if (*fmt == '%') {
		    done = TRUE;
		}
		if (used != atUnknown && parms < MAX_PARMS) {
		    parm[parms++] = used;
		    for (n = 0; n < parms; ++n) {
			used = parm[n];
			param = buffer;
			switch (used) {
			case atInteger:
			    sprintf(buffer, "%d", ival);
			    break;
			case atFloat:
			    sprintf(buffer, "%f", fval);
			    break;
			case atPoint:
			    sprintf(buffer, "%p", pval);
			    break;
			case atString:
			    param = _nc_visbuf2(1, sval);
			    break;
			case atUnknown:
			default:
			    strcpy(buffer, "?");
			    break;
			}
			MyLength += strlen(param) + 2;
			MyBuffer = typeRealloc(char, MyLength, MyBuffer);
			sprintf(MyBuffer + strlen(MyBuffer), ", %s", param);
		    }
		}
		used = atUnknown;
	    }
	} else {
	    fmt++;
	}
    }

    return (MyBuffer);
}
#else
EMPTY_MODULE(_nc_varargs)
#endif