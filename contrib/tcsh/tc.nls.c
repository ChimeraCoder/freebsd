
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
#include "sh.h"

RCSID("$tcsh: tc.nls.c,v 3.23 2010/02/12 22:17:20 christos Exp $")


#ifdef WIDE_STRINGS
# ifdef HAVE_WCWIDTH
#  ifdef UTF16_STRINGS
int
xwcwidth (wint_t wchar)
{
  wchar_t ws[2];

  if (wchar <= 0xffff)
    return wcwidth ((wchar_t) wchar);
  /* UTF-16 systems can't handle these values directly in calls to wcwidth.
     However, they can handle them as surrogate pairs in calls to wcswidth.
     What we do here is to convert UTF-32 values >= 0x10000 into surrogate
     pairs and compute the width by calling wcswidth. */
  wchar -= 0x10000;
  ws[0] = 0xd800 | (wchar >> 10);
  ws[1] = 0xdc00 | (wchar & 0x3ff);
  return wcswidth (ws, 2);
}
#  else
#define xwcwidth wcwidth
#  endif /* !UTF16_STRINGS */
# endif /* HAVE_WCWIDTH */

int
NLSWidth(Char c)
{
# ifdef HAVE_WCWIDTH
    int l;
    if (c & INVALID_BYTE)
	return 1;
    l = xwcwidth((wchar_t) c);
    return l >= 0 ? l : 0;
# else
    return iswprint(c) != 0;
# endif
}

int
NLSStringWidth(const Char *s)
{
    int w = 0, l;
    Char c;

    while (*s) {
	c = *s++;
#ifdef HAVE_WCWIDTH
	if ((l = xwcwidth((wchar_t) c)) < 0)
		l = 2;
#else
	l = iswprint(c) != 0;
#endif
	w += l;
    }
    return w;
}
#endif

Char *
NLSChangeCase(const Char *p, int mode)
{
    Char c, *n, c2 = 0;
    const Char *op = p;

    for (; (c = *p) != 0; p++) {
        if (mode == 0 && Islower(c)) {
	    c2 = Toupper(c);
	    break;
        } else if (mode && Isupper(c)) {
	    c2 = Tolower(c);
	    break;
	}
    }
    if (!*p)
	return 0;
    n = Strsave(op);
    n[p - op] = c2;
    return n;
}

int
NLSClassify(Char c, int nocomb)
{
    int w;
    if (c & INVALID_BYTE)
	return NLSCLASS_ILLEGAL;
    w = NLSWidth(c);
    if ((w > 0 && !(Iscntrl(c) && (c & CHAR) < 0x100)) || (Isprint(c) && !nocomb))
	return w;
    if (Iscntrl(c) && (c & CHAR) < 0x100) {
	if (c == '\n')
	    return NLSCLASS_NL;
	if (c == '\t')
	    return NLSCLASS_TAB;
	return NLSCLASS_CTRL;
    }
#ifdef WIDE_STRINGS
    if (c >= 0x1000000)
	return NLSCLASS_ILLEGAL4;
    if (c >= 0x10000)
	return NLSCLASS_ILLEGAL3;
#endif
    if (c >= 0x100)
	return NLSCLASS_ILLEGAL2;
    return NLSCLASS_ILLEGAL;
}