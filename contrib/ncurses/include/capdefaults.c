
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

/* $Id: capdefaults.c,v 1.13 2008/08/04 12:33:42 tom Exp $ */

    /*
     * Compute obsolete capabilities.  The reason this is an include file is
     * that the two places where it's needed want the macros to generate
     * offsets to different structures.  See the file Caps for explanations of
     * these conversions.
     *
     * Note:  This code is the functional inverse of the first part of
     * postprocess_termcap().
     */
{
    char *sp;
    short capval;

#define EXTRACT_DELAY(str) \
    	(short) (sp = strchr(str, '*'), sp ? atoi(sp+1) : 0)

    /* current (4.4BSD) capabilities marked obsolete */
    if (VALID_STRING(carriage_return)
	&& (capval = EXTRACT_DELAY(carriage_return)))
	carriage_return_delay = capval;
    if (VALID_STRING(newline) && (capval = EXTRACT_DELAY(newline)))
	new_line_delay = capval;

    /* current (4.4BSD) capabilities not obsolete */
    if (!VALID_STRING(termcap_init2) && VALID_STRING(init_3string)) {
	termcap_init2 = init_3string;
	init_3string = ABSENT_STRING;
    }
    if (!VALID_STRING(termcap_reset)
     && VALID_STRING(reset_2string)
     && !VALID_STRING(reset_1string)
     && !VALID_STRING(reset_3string)) {
	termcap_reset = reset_2string;
	reset_2string = ABSENT_STRING;
    }
    if (magic_cookie_glitch_ul == ABSENT_NUMERIC
	&& magic_cookie_glitch != ABSENT_NUMERIC
	&& VALID_STRING(enter_underline_mode))
	magic_cookie_glitch_ul = magic_cookie_glitch;

    /* totally obsolete capabilities */
    linefeed_is_newline = (char) (VALID_STRING(newline)
				  && (strcmp("\n", newline) == 0));
    if (VALID_STRING(cursor_left)
	&& (capval = EXTRACT_DELAY(cursor_left)))
	backspace_delay = capval;
    if (VALID_STRING(tab) && (capval = EXTRACT_DELAY(tab)))
	horizontal_tab_delay = capval;
#undef EXTRACT_DELAY
}