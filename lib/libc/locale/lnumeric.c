
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

#include <limits.h>

#include "ldpart.h"
#include "lnumeric.h"

extern const char *__fix_locale_grouping_str(const char *);

#define LCNUMERIC_SIZE (sizeof(struct lc_numeric_T) / sizeof(char *))

static char	numempty[] = { CHAR_MAX, '\0' };

static const struct lc_numeric_T _C_numeric_locale = {
	".",     	/* decimal_point */
	"",     	/* thousands_sep */
	numempty	/* grouping */
};

static void
destruct_numeric(void *v)
{
	struct xlocale_numeric *l = v;
	if (l->buffer)
		free(l->buffer);
	free(l);
}

struct xlocale_numeric __xlocale_global_numeric;

static int
numeric_load_locale(struct xlocale_numeric *loc, int *using_locale, int *changed,
		const char *name)
{
	int ret;
	struct lc_numeric_T *l = &loc->locale;

	ret = __part_load_locale(name, using_locale,
		&loc->buffer, "LC_NUMERIC",
		LCNUMERIC_SIZE, LCNUMERIC_SIZE,
		(const char**)l);
	if (ret != _LDP_ERROR)
		*changed= 1;
	if (ret == _LDP_LOADED) {
		/* Can't be empty according to C99 */
		if (*l->decimal_point == '\0')
			l->decimal_point =
			    _C_numeric_locale.decimal_point;
		l->grouping =
		    __fix_locale_grouping_str(l->grouping);
	}
	return (ret);
}

int
__numeric_load_locale(const char *name)
{
	return numeric_load_locale(&__xlocale_global_numeric,
			&__xlocale_global_locale.using_numeric_locale,
			&__xlocale_global_locale.numeric_locale_changed, name);
}
void *
__numeric_load(const char *name, locale_t l)
{
	struct xlocale_numeric *new = calloc(sizeof(struct xlocale_numeric), 1);
	new->header.header.destructor = destruct_numeric;
	if (numeric_load_locale(new, &l->using_numeric_locale,
				&l->numeric_locale_changed, name) == _LDP_ERROR)
	{
		xlocale_release(new);
		return NULL;
	}
	return new;
}

struct lc_numeric_T *
__get_current_numeric_locale(locale_t loc)
{
	return (loc->using_numeric_locale
		? &((struct xlocale_numeric *)loc->components[XLC_NUMERIC])->locale
		: (struct lc_numeric_T *)&_C_numeric_locale);
}

#ifdef LOCALE_DEBUG
void
numericdebug(void) {
printf(	"decimal_point = %s\n"
	"thousands_sep = %s\n"
	"grouping = %s\n",
	_numeric_locale.decimal_point,
	_numeric_locale.thousands_sep,
	_numeric_locale.grouping
);
}
#endif /* LOCALE_DEBUG */