
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

#include <langinfo.h>
#include <limits.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include "lnumeric.h"
#include "lmessages.h"
#include "lmonetary.h"
#include "../stdtime/timelocal.h"

#define _REL(BASE) ((int)item-BASE)

char *
nl_langinfo_l(nl_item item, locale_t loc)
{
   char *ret, *cs;
   const char *s;
   FIX_LOCALE(loc);

   switch (item) {
	case CODESET:
		ret = "";
		if ((s = querylocale(LC_CTYPE_MASK, loc)) != NULL) {
			if ((cs = strchr(s, '.')) != NULL)
				ret = cs + 1;
			else if (strcmp(s, "C") == 0 ||
				 strcmp(s, "POSIX") == 0)
				ret = "US-ASCII";
		}
		break;
	case D_T_FMT:
		ret = (char *) __get_current_time_locale(loc)->c_fmt;
		break;
	case D_FMT:
		ret = (char *) __get_current_time_locale(loc)->x_fmt;
		break;
	case T_FMT:
		ret = (char *) __get_current_time_locale(loc)->X_fmt;
		break;
	case T_FMT_AMPM:
		ret = (char *) __get_current_time_locale(loc)->ampm_fmt;
		break;
	case AM_STR:
		ret = (char *) __get_current_time_locale(loc)->am;
		break;
	case PM_STR:
		ret = (char *) __get_current_time_locale(loc)->pm;
		break;
	case DAY_1: case DAY_2: case DAY_3:
	case DAY_4: case DAY_5: case DAY_6: case DAY_7:
		ret = (char*) __get_current_time_locale(loc)->weekday[_REL(DAY_1)];
		break;
	case ABDAY_1: case ABDAY_2: case ABDAY_3:
	case ABDAY_4: case ABDAY_5: case ABDAY_6: case ABDAY_7:
		ret = (char*) __get_current_time_locale(loc)->wday[_REL(ABDAY_1)];
		break;
	case MON_1: case MON_2: case MON_3: case MON_4:
	case MON_5: case MON_6: case MON_7: case MON_8:
	case MON_9: case MON_10: case MON_11: case MON_12:
		ret = (char*) __get_current_time_locale(loc)->month[_REL(MON_1)];
		break;
	case ABMON_1: case ABMON_2: case ABMON_3: case ABMON_4:
	case ABMON_5: case ABMON_6: case ABMON_7: case ABMON_8:
	case ABMON_9: case ABMON_10: case ABMON_11: case ABMON_12:
		ret = (char*) __get_current_time_locale(loc)->mon[_REL(ABMON_1)];
		break;
	case ALTMON_1: case ALTMON_2: case ALTMON_3: case ALTMON_4:
	case ALTMON_5: case ALTMON_6: case ALTMON_7: case ALTMON_8:
	case ALTMON_9: case ALTMON_10: case ALTMON_11: case ALTMON_12:
		ret = (char*)
		    __get_current_time_locale(loc)->alt_month[_REL(ALTMON_1)];
		break;
	case ERA:
		/* XXX: need to be implemented  */
		ret = "";
		break;
	case ERA_D_FMT:
		/* XXX: need to be implemented  */
		ret = "";
		break;
	case ERA_D_T_FMT:
		/* XXX: need to be implemented  */
		ret = "";
		break;
	case ERA_T_FMT:
		/* XXX: need to be implemented  */
		ret = "";
		break;
	case ALT_DIGITS:
		/* XXX: need to be implemented  */
		ret = "";
		break;
	case RADIXCHAR:
		ret = (char*) __get_current_numeric_locale(loc)->decimal_point;
		break;
	case THOUSEP:
		ret = (char*) __get_current_numeric_locale(loc)->thousands_sep;
		break;
	case YESEXPR:
		ret = (char*) __get_current_messages_locale(loc)->yesexpr;
		break;
	case NOEXPR:
		ret = (char*) __get_current_messages_locale(loc)->noexpr;
		break;
	/*
	 * YESSTR and NOSTR items marked with LEGACY are available, but not
	 * recomended by SUSv2 to be used in portable applications since
	 * they're subject to remove in future specification editions.
	 */
	case YESSTR:            /* LEGACY  */
		ret = (char*) __get_current_messages_locale(loc)->yesstr;
		break;
	case NOSTR:             /* LEGACY  */
		ret = (char*) __get_current_messages_locale(loc)->nostr;
		break;
	/*
	 * SUSv2 special formatted currency string 
	 */
	case CRNCYSTR:
		ret = "";
		cs = (char*) __get_current_monetary_locale(loc)->currency_symbol;
		if (*cs != '\0') {
			char pos = localeconv_l(loc)->p_cs_precedes;

			if (pos == localeconv_l(loc)->n_cs_precedes) {
				char psn = '\0';

				if (pos == CHAR_MAX) {
					if (strcmp(cs, __get_current_monetary_locale(loc)->mon_decimal_point) == 0)
						psn = '.';
				} else
					psn = pos ? '-' : '+';
				if (psn != '\0') {
					int clen = strlen(cs);

					if ((loc->csym = reallocf(loc->csym, clen + 2)) != NULL) {
						*loc->csym = psn;
						strcpy(loc->csym + 1, cs);
						ret = loc->csym;
					}
				}
			}
		}
		break;
	case D_MD_ORDER:        /* FreeBSD local extension */
		ret = (char *) __get_current_time_locale(loc)->md_order;
		break;
	default:
		ret = "";
   }
   return (ret);
}

char *
nl_langinfo(nl_item item)
{
	return nl_langinfo_l(item, __get_locale());
}