
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)localeconv.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <locale.h>

#include "lmonetary.h"
#include "lnumeric.h"

/* 
 * The localeconv() function constructs a struct lconv from the current
 * monetary and numeric locales.
 *
 * Because localeconv() may be called many times (especially by library
 * routines like printf() & strtod()), the approprate members of the 
 * lconv structure are computed only when the monetary or numeric 
 * locale has been changed.
 */

/*
 * Return the current locale conversion.
 */
struct lconv *
localeconv_l(locale_t loc)
{
	FIX_LOCALE(loc);
    struct lconv *ret = &loc->lconv;

    if (loc->monetary_locale_changed) {
	/* LC_MONETARY part */
        struct lc_monetary_T * mptr; 

#define M_ASSIGN_STR(NAME) (ret->NAME = (char*)mptr->NAME)
#define M_ASSIGN_CHAR(NAME) (ret->NAME = mptr->NAME[0])

	mptr = __get_current_monetary_locale(loc);
	M_ASSIGN_STR(int_curr_symbol);
	M_ASSIGN_STR(currency_symbol);
	M_ASSIGN_STR(mon_decimal_point);
	M_ASSIGN_STR(mon_thousands_sep);
	M_ASSIGN_STR(mon_grouping);
	M_ASSIGN_STR(positive_sign);
	M_ASSIGN_STR(negative_sign);
	M_ASSIGN_CHAR(int_frac_digits);
	M_ASSIGN_CHAR(frac_digits);
	M_ASSIGN_CHAR(p_cs_precedes);
	M_ASSIGN_CHAR(p_sep_by_space);
	M_ASSIGN_CHAR(n_cs_precedes);
	M_ASSIGN_CHAR(n_sep_by_space);
	M_ASSIGN_CHAR(p_sign_posn);
	M_ASSIGN_CHAR(n_sign_posn);
	M_ASSIGN_CHAR(int_p_cs_precedes);
	M_ASSIGN_CHAR(int_n_cs_precedes);
	M_ASSIGN_CHAR(int_p_sep_by_space);
	M_ASSIGN_CHAR(int_n_sep_by_space);
	M_ASSIGN_CHAR(int_p_sign_posn);
	M_ASSIGN_CHAR(int_n_sign_posn);
	loc->monetary_locale_changed = 0;
    }

    if (loc->numeric_locale_changed) {
	/* LC_NUMERIC part */
        struct lc_numeric_T * nptr; 

#define N_ASSIGN_STR(NAME) (ret->NAME = (char*)nptr->NAME)

	nptr = __get_current_numeric_locale(loc);
	N_ASSIGN_STR(decimal_point);
	N_ASSIGN_STR(thousands_sep);
	N_ASSIGN_STR(grouping);
	loc->numeric_locale_changed = 0;
    }

    return ret;
}
struct lconv *
localeconv(void)
{
	return localeconv_l(__get_locale());
}