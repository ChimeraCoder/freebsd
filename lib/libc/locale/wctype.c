
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

#include <ctype.h>
#include <string.h>
#include <wctype.h>
#include <xlocale.h>

#undef iswctype
int
iswctype(wint_t wc, wctype_t charclass)
{
	return (__istype(wc, charclass));
}
int
iswctype_l(wint_t wc, wctype_t charclass, locale_t locale)
{
	return __istype_l(wc, charclass, locale);
}

/*
 * IMPORTANT: The 0 in the call to this function in wctype() must be changed to
 * __get_locale() if wctype_l() is ever modified to actually use the locale
 * parameter.
 */
wctype_t
wctype_l(const char *property, locale_t locale)
{
	const char *propnames = 
		"alnum\0"
		"alpha\0"
		"blank\0"
		"cntrl\0"
		"digit\0"
		"graph\0"
		"lower\0"
		"print\0"
		"punct\0"
		"space\0"
		"upper\0"
		"xdigit\0"
		"ideogram\0"	/* BSD extension */
		"special\0"	/* BSD extension */
		"phonogram\0"	/* BSD extension */
		"rune\0";	/* BSD extension */
	static const wctype_t propmasks[] = {
		_CTYPE_A|_CTYPE_D,
		_CTYPE_A,
		_CTYPE_B,
		_CTYPE_C,
		_CTYPE_D,
		_CTYPE_G,
		_CTYPE_L,
		_CTYPE_R,
		_CTYPE_P,
		_CTYPE_S,
		_CTYPE_U,
		_CTYPE_X,
		_CTYPE_I,
		_CTYPE_T,
		_CTYPE_Q,
		0xFFFFFF00L
	};
	size_t len1, len2;
	const char *p;
	const wctype_t *q;

	len1 = strlen(property);
	q = propmasks;
	for (p = propnames; (len2 = strlen(p)) != 0; p += len2 + 1) {
		if (len1 == len2 && memcmp(property, p, len1) == 0)
			return (*q);
		q++;
	}

	return (0UL);
}

wctype_t wctype(const char *property)
{
	return wctype_l(property, 0);
}