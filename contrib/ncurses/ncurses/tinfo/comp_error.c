
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
 *  Author: Zeyd M. Ben-Halim <zmbenhal@netcom.com> 1992,1995               *
 *     and: Eric S. Raymond <esr@snark.thyrsus.com>                         *
 *     and: Thomas E. Dickey                        1996-on                 *
 ****************************************************************************/

/*
 *	comp_error.c -- Error message routines
 *
 */

#include <curses.priv.h>

#include <tic.h>

MODULE_ID("$Id: comp_error.c,v 1.31 2007/04/21 23:38:32 tom Exp $")

NCURSES_EXPORT_VAR(bool) _nc_suppress_warnings = FALSE;
NCURSES_EXPORT_VAR(int) _nc_curr_line = 0; /* current line # in input */
NCURSES_EXPORT_VAR(int) _nc_curr_col = 0; /* current column # in input */

#define SourceName	_nc_globals.comp_sourcename
#define TermType	_nc_globals.comp_termtype

NCURSES_EXPORT(const char *)
_nc_get_source(void)
{
    return SourceName;
}

NCURSES_EXPORT(void)
_nc_set_source(const char *const name)
{
    SourceName = name;
}

NCURSES_EXPORT(void)
_nc_set_type(const char *const name)
{
    if (TermType == 0)
	TermType = typeMalloc(char, MAX_NAME_SIZE + 1);
    if (TermType != 0) {
	TermType[0] = '\0';
	if (name)
	    strncat(TermType, name, MAX_NAME_SIZE);
    }
}

NCURSES_EXPORT(void)
_nc_get_type(char *name)
{
#if NO_LEAKS
    if (name == 0 && TermType != 0) {
	FreeAndNull(TermType);
	return;
    }
#endif
    if (name != 0)
	strcpy(name, TermType != 0 ? TermType : "");
}

static NCURSES_INLINE void
where_is_problem(void)
{
    fprintf(stderr, "\"%s\"", SourceName ? SourceName : "?");
    if (_nc_curr_line >= 0)
	fprintf(stderr, ", line %d", _nc_curr_line);
    if (_nc_curr_col >= 0)
	fprintf(stderr, ", col %d", _nc_curr_col);
    if (TermType != 0 && TermType[0] != '\0')
	fprintf(stderr, ", terminal '%s'", TermType);
    fputc(':', stderr);
    fputc(' ', stderr);
}

NCURSES_EXPORT(void)
_nc_warning(const char *const fmt,...)
{
    va_list argp;

    if (_nc_suppress_warnings)
	return;

    where_is_problem();
    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    fprintf(stderr, "\n");
    va_end(argp);
}

NCURSES_EXPORT(void)
_nc_err_abort(const char *const fmt,...)
{
    va_list argp;

    where_is_problem();
    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    fprintf(stderr, "\n");
    va_end(argp);
    exit(EXIT_FAILURE);
}

NCURSES_EXPORT(void)
_nc_syserr_abort(const char *const fmt,...)
{
    va_list argp;

    where_is_problem();
    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    fprintf(stderr, "\n");
    va_end(argp);

    /* If we're debugging, try to show where the problem occurred - this
     * will dump core.
     */
#if defined(TRACE) || !defined(NDEBUG)
    abort();
#else
    /* Dumping core in production code is not a good idea.
     */
    exit(EXIT_FAILURE);
#endif
}