
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
#include <limits.h>
#include <stddef.h>

static const char nogrouping[] = { CHAR_MAX, '\0' };

/*
 * Internal helper used to convert grouping sequences from string
 * representation into POSIX specified form, i.e.
 *
 * "3;3;-1" -> "\003\003\177\000"
 */

const char *
__fix_locale_grouping_str(const char *str)
{
	char *src, *dst;
	char n;

	if (str == NULL || *str == '\0') {
		return nogrouping;
	}

	for (src = (char*)str, dst = (char*)str; *src != '\0'; src++) {

		/* input string examples: "3;3", "3;2;-1" */
		if (*src == ';')
			continue;
	
		if (*src == '-' && *(src+1) == '1') {
			*dst++ = CHAR_MAX;
			src++;
			continue;
		}

		if (!isdigit((unsigned char)*src)) {
			/* broken grouping string */
			return nogrouping;
		}

		/* assume all numbers <= 99 */
		n = *src - '0';
		if (isdigit((unsigned char)*(src+1))) {
			src++;
			n *= 10;
			n += *src - '0';
		}

		*dst = n;
		/* NOTE: assume all input started with "0" as 'no grouping' */
		if (*dst == '\0')
			return (dst == (char*)str) ? nogrouping : str;
		dst++;
	}
	*dst = '\0';
	return str;
}