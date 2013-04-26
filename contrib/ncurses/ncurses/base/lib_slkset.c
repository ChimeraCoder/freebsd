
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
 *  Author: Juergen Pfeifer                                                 *
 *     and: Thomas E. Dickey                                                *
 ****************************************************************************/

/*
 *	lib_slkset.c
 *      Set soft label text.
 */
#include <curses.priv.h>
#include <ctype.h>

#if USE_WIDEC_SUPPORT
#if HAVE_WCTYPE_H
#include <wctype.h>
#endif
#endif

MODULE_ID("$Id: lib_slkset.c,v 1.17 2007/10/13 20:08:46 tom Exp $")

NCURSES_EXPORT(int)
slk_set(int i, const char *astr, int format)
{
    SLK *slk;
    int offset;
    int numchrs;
    int numcols;
    int limit;
    const char *str = astr;
    const char *p;

    T((T_CALLED("slk_set(%d, \"%s\", %d)"), i, str, format));

    if (SP == 0
	|| (slk = SP->_slk) == 0
	|| i < 1
	|| i > slk->labcnt
	|| format < 0
	|| format > 2)
	returnCode(ERR);
    if (str == NULL)
	str = "";
    --i;			/* Adjust numbering of labels */

    limit = MAX_SKEY_LEN(SP->slk_format);
    while (isspace(UChar(*str)))
	str++;			/* skip over leading spaces  */
    p = str;

#if USE_WIDEC_SUPPORT
    numcols = 0;
    while (*p != 0) {
	mbstate_t state;
	wchar_t wc;
	size_t need;

	init_mb(state);
	need = mbrtowc(0, p, strlen(p), &state);
	if (need == (size_t) -1)
	    break;
	mbrtowc(&wc, p, need, &state);
	if (!iswprint((wint_t) wc))
	    break;
	if (wcwidth(wc) + numcols > limit)
	    break;
	numcols += wcwidth(wc);
	p += need;
    }
    numchrs = (p - str);
#else
    while (isprint(UChar(*p)))
	p++;			/* The first non-print stops */

    numcols = (p - str);
    if (numcols > limit)
	numcols = limit;
    numchrs = numcols;
#endif

    FreeIfNeeded(slk->ent[i].ent_text);
    if ((slk->ent[i].ent_text = strdup(str)) == 0)
	returnCode(ERR);
    slk->ent[i].ent_text[numchrs] = '\0';

    if ((slk->ent[i].form_text = (char *) _nc_doalloc(slk->ent[i].form_text,
						      (unsigned) (limit +
								  numchrs + 1))
	) == 0)
	returnCode(ERR);

    switch (format) {
    default:
    case 0:			/* left-justified */
	offset = 0;
	break;
    case 1:			/* centered */
	offset = (limit - numcols) / 2;
	break;
    case 2:			/* right-justified */
	offset = limit - numcols;
	break;
    }
    if (offset <= 0)
	offset = 0;
    else
	memset(slk->ent[i].form_text, ' ', (unsigned) offset);

    memcpy(slk->ent[i].form_text + offset,
	   slk->ent[i].ent_text,
	   (unsigned) numchrs);

    if (offset < limit) {
	memset(slk->ent[i].form_text + offset + numchrs,
	       ' ',
	       (unsigned) (limit - (offset + numcols)));
    }

    slk->ent[i].form_text[numchrs - numcols + limit] = 0;
    slk->ent[i].dirty = TRUE;
    returnCode(OK);
}