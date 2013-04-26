
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

#include <stdlib.h>
#include <string.h>
#include "collate.h"

#include <stdio.h>

int
strcoll_l(const char *s, const char *s2, locale_t locale)
{
	int len, len2, prim, prim2, sec, sec2, ret, ret2;
	const char *t, *t2;
	char *tt, *tt2;
	FIX_LOCALE(locale);
	struct xlocale_collate *table =
		(struct xlocale_collate*)locale->components[XLC_COLLATE];

	if (table->__collate_load_error)
		return strcmp(s, s2);

	len = len2 = 1;
	ret = ret2 = 0;
	if (table->__collate_substitute_nontrivial) {
		t = tt = __collate_substitute(table, s);
		t2 = tt2 = __collate_substitute(table, s2);
	} else {
		tt = tt2 = NULL;
		t = s;
		t2 = s2;
	}
	while(*t && *t2) {
		prim = prim2 = 0;
		while(*t && !prim) {
			__collate_lookup(table, t, &len, &prim, &sec);
			t += len;
		}
		while(*t2 && !prim2) {
			__collate_lookup(table, t2, &len2, &prim2, &sec2);
			t2 += len2;
		}
		if(!prim || !prim2)
			break;
		if(prim != prim2) {
			ret = prim - prim2;
			goto end;
		}
		if(!ret2)
			ret2 = sec - sec2;
	}
	if(!*t && *t2)
		ret = -(int)((u_char)*t2);
	else if(*t && !*t2)
		ret = (u_char)*t;
	else if(!*t && !*t2)
		ret = ret2;
  end:
	free(tt);
	free(tt2);

	return ret;
}

int
strcoll(const char *s, const char *s2)
{
	return strcoll_l(s, s2, __get_locale());
}