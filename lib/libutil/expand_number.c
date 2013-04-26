
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

#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <libutil.h>
#include <stdint.h>

/*
 * Convert an expression of the following forms to a uint64_t.
 *	1) A positive decimal number.
 *	2) A positive decimal number followed by a 'b' or 'B' (mult by 1).
 *	3) A positive decimal number followed by a 'k' or 'K' (mult by 1 << 10).
 *	4) A positive decimal number followed by a 'm' or 'M' (mult by 1 << 20).
 *	5) A positive decimal number followed by a 'g' or 'G' (mult by 1 << 30).
 *	6) A positive decimal number followed by a 't' or 'T' (mult by 1 << 40).
 *	7) A positive decimal number followed by a 'p' or 'P' (mult by 1 << 50).
 *	8) A positive decimal number followed by a 'e' or 'E' (mult by 1 << 60).
 */
int
expand_number(const char *buf, uint64_t *num)
{
	uint64_t number;
	unsigned shift;
	char *endptr;

	number = strtoumax(buf, &endptr, 0);

	if (endptr == buf) {
		/* No valid digits. */
		errno = EINVAL;
		return (-1);
	}

	switch (tolower((unsigned char)*endptr)) {
	case 'e':
		shift = 60;
		break;
	case 'p':
		shift = 50;
		break;
	case 't':
		shift = 40;
		break;
	case 'g':
		shift = 30;
		break;
	case 'm':
		shift = 20;
		break;
	case 'k':
		shift = 10;
		break;
	case 'b':
	case '\0': /* No unit. */
		*num = number;
		return (0);
	default:
		/* Unrecognized unit. */
		errno = EINVAL;
		return (-1);
	}

	if ((number << shift) >> shift != number) {
		/* Overflow */
		errno = ERANGE;
		return (-1);
	}

	*num = number << shift;
	return (0);
}