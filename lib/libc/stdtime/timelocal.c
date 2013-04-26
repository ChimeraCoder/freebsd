
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

#include <stddef.h>

#include "ldpart.h"
#include "timelocal.h"

struct xlocale_time {
	struct xlocale_component header;
	char *buffer;
	struct lc_time_T locale;
};

struct xlocale_time __xlocale_global_time;

#define LCTIME_SIZE (sizeof(struct lc_time_T) / sizeof(char *))

static const struct lc_time_T	_C_time_locale = {
	{
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	}, {
		"January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October", "November", "December"
	}, {
		"Sun", "Mon", "Tue", "Wed",
		"Thu", "Fri", "Sat"
	}, {
		"Sunday", "Monday", "Tuesday", "Wednesday",
		"Thursday", "Friday", "Saturday"
	},

	/* X_fmt */
	"%H:%M:%S",

	/*
	 * x_fmt
	 * Since the C language standard calls for
	 * "date, using locale's date format," anything goes.
	 * Using just numbers (as here) makes Quakers happier;
	 * it's also compatible with SVR4.
	 */
	"%m/%d/%y",

	/*
	 * c_fmt
	 */
	"%a %b %e %H:%M:%S %Y",

	/* am */
	"AM",

	/* pm */
	"PM",

	/* date_fmt */
	"%a %b %e %H:%M:%S %Z %Y",
	
	/* alt_month
	 * Standalone months forms for %OB
	 */
	{
		"January", "February", "March", "April", "May", "June",
		"July", "August", "September", "October", "November", "December"
	},

	/* md_order
	 * Month / day order in dates
	 */
	"md",

	/* ampm_fmt
	 * To determine 12-hour clock format time (empty, if N/A)
	 */
	"%I:%M:%S %p"
};

static void destruct_time(void *v)
{
	struct xlocale_time *l = v;
	if (l->buffer)
		free(l->buffer);
	free(l);
}

#include <stdio.h>
struct lc_time_T *
__get_current_time_locale(locale_t loc)
{
	return (loc->using_time_locale
		? &((struct xlocale_time *)loc->components[XLC_TIME])->locale
		: (struct lc_time_T *)&_C_time_locale);
}

static int
time_load_locale(struct xlocale_time *l, int *using_locale, const char *name)
{
	struct lc_time_T *time_locale = &l->locale;
	return (__part_load_locale(name, using_locale,
			&l->buffer, "LC_TIME",
			LCTIME_SIZE, LCTIME_SIZE,
			(const char **)time_locale));
}
int
__time_load_locale(const char *name)
{
	return time_load_locale(&__xlocale_global_time,
			&__xlocale_global_locale.using_time_locale, name);
}
void* __time_load(const char* name, locale_t loc)
{
	struct xlocale_time *new = calloc(sizeof(struct xlocale_time), 1);
	new->header.header.destructor = destruct_time;
	if (time_load_locale(new, &loc->using_time_locale, name) == _LDP_ERROR)
	{
		xlocale_release(new);
		return NULL;
	}
	return new;
}