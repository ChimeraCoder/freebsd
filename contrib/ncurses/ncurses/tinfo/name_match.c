
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
 *  Author: Thomas E. Dickey                    1999-on                     *
 ****************************************************************************/

#include <curses.priv.h>
#include <term.h>
#include <tic.h>

MODULE_ID("$Id: name_match.c,v 1.17 2008/08/03 19:49:33 tom Exp $")

/*
 *	_nc_first_name(char *names)
 *
 *	Extract the primary name from a compiled entry.
 */
#define FirstName _nc_globals.first_name

NCURSES_EXPORT(char *)
_nc_first_name(const char *const sp)
/* get the first name from the given name list */
{
    unsigned n;

#if NO_LEAKS
    if (sp == 0) {
	if (FirstName != 0)
	    FreeAndNull(FirstName);
    } else
#endif
    {
	if (FirstName == 0)
	    FirstName = typeMalloc(char, MAX_NAME_SIZE + 1);

	if (FirstName != 0) {
	    for (n = 0; n < MAX_NAME_SIZE; n++) {
		if ((FirstName[n] = sp[n]) == '\0'
		    || (FirstName[n] == '|'))
		    break;
	    }
	    FirstName[n] = '\0';
	}
    }
    return (FirstName);
}

/*
 *	int _nc_name_match(namelist, name, delim)
 *
 *	Is the given name matched in namelist?
 */

NCURSES_EXPORT(int)
_nc_name_match(const char *const namelst, const char *const name, const char *const delim)
{
    const char *s, *d, *t;
    int code, found;

    if ((s = namelst) != 0) {
	while (*s != '\0') {
	    for (d = name; *d != '\0'; d++) {
		if (*s != *d)
		    break;
		s++;
	    }
	    found = FALSE;
	    for (code = TRUE; *s != '\0'; code = FALSE, s++) {
		for (t = delim; *t != '\0'; t++) {
		    if (*s == *t) {
			found = TRUE;
			break;
		    }
		}
		if (found)
		    break;
	    }
	    if (code && *d == '\0')
		return code;
	    if (*s++ == 0)
		break;
	}
    }
    return FALSE;
}