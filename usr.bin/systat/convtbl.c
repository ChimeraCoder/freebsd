
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

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include "convtbl.h"

#define BIT		(8)
#define BITS		(1)
#define KILOBIT		(1000LL)
#define MEGABIT		(KILOBIT * 1000)
#define GIGABIT		(MEGABIT * 1000)
#define TERABIT		(GIGABIT * 1000)

#define BYTE		(1)
#define BYTES		(1)
#define KILOBYTE	(1024LL)
#define MEGABYTE	(KILOBYTE * 1024)
#define GIGABYTE	(MEGABYTE * 1024)
#define TERABYTE	(GIGABYTE * 1024)

struct convtbl {
	uintmax_t	 mul;
	uintmax_t	 scale;
	const char	*str;
	const char	*name;
};

static struct convtbl convtbl[] = {
	/* mul, scale, str, name */
	[SC_BYTE] =	{ BYTE, BYTES, "B", "byte" },
	[SC_KILOBYTE] =	{ BYTE, KILOBYTE, "KB", "kbyte" },
	[SC_MEGABYTE] =	{ BYTE, MEGABYTE, "MB", "mbyte" },
	[SC_GIGABYTE] =	{ BYTE, GIGABYTE, "GB", "gbyte" },
	[SC_TERABYTE] =	{ BYTE, TERABYTE, "TB", "tbyte" },

	[SC_BIT] =	{ BIT, BITS, "b", "bit" },
	[SC_KILOBIT] =	{ BIT, KILOBIT, "Kb", "kbit" },
	[SC_MEGABIT] =	{ BIT, MEGABIT, "Mb", "mbit" },
	[SC_GIGABIT] =	{ BIT, GIGABIT, "Gb", "gbit" },
	[SC_TERABIT] =	{ BIT, TERABIT, "Tb", "tbit" },

	[SC_AUTO] =	{ 0, 0, "", "auto" }
};

static
struct convtbl *
get_tbl_ptr(const uintmax_t size, const int scale)
{
	uintmax_t	 tmp;
	int		 idx;

	/* If our index is out of range, default to auto-scaling. */
	idx = scale < SC_AUTO ? scale : SC_AUTO;

	if (idx == SC_AUTO)
		/*
		 * Simple but elegant algorithm.  Count how many times
		 * we can shift our size value right by a factor of ten,
		 * incrementing an index each time.  We then use the
		 * index as the array index into the conversion table.
		 */
		for (tmp = size, idx = SC_KILOBYTE;
		     tmp >= MEGABYTE && idx < SC_BIT - 1;
		     tmp >>= 10, idx++);

	return (&convtbl[idx]);
}

double
convert(const uintmax_t size, const int scale)
{
	struct convtbl	*tp;

	tp = get_tbl_ptr(size, scale);
	return ((double)size * tp->mul / tp->scale);

}

const char *
get_string(const uintmax_t size, const int scale)
{
	struct convtbl	*tp;

	tp = get_tbl_ptr(size, scale);
	return (tp->str);
}

int
get_scale(const char *name)
{
	int i;

	for (i = 0; i <= SC_AUTO; i++)
		if (strcmp(convtbl[i].name, name) == 0)
			return (i);
	return (-1);
}

const char *
get_helplist(void)
{
	int i;
	size_t len;
	static char *buf;

	if (buf == NULL) {
		len = 0;
		for (i = 0; i <= SC_AUTO; i++)
			len += strlen(convtbl[i].name) + 2;
		if ((buf = malloc(len)) != NULL) {
			buf[0] = '\0';
			for (i = 0; i <= SC_AUTO; i++) {
				strcat(buf, convtbl[i].name);
				if (i < SC_AUTO)
					strcat(buf, ", ");
			}
		} else
			return ("");
	}
	return (buf);
}