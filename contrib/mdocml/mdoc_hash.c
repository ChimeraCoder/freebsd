
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mdoc.h"
#include "mandoc.h"
#include "libmdoc.h"

static	unsigned char	 table[27 * 12];

/*
 * XXX - this hash has global scope, so if intended for use as a library
 * with multiple callers, it will need re-invocation protection.
 */
void
mdoc_hash_init(void)
{
	int		 i, j, major;
	const char	*p;

	memset(table, UCHAR_MAX, sizeof(table));

	for (i = 0; i < (int)MDOC_MAX; i++) {
		p = mdoc_macronames[i];

		if (isalpha((unsigned char)p[1]))
			major = 12 * (tolower((unsigned char)p[1]) - 97);
		else
			major = 12 * 26;

		for (j = 0; j < 12; j++)
			if (UCHAR_MAX == table[major + j]) {
				table[major + j] = (unsigned char)i;
				break;
			}

		assert(j < 12);
	}
}

enum mdoct
mdoc_hash_find(const char *p)
{
	int		  major, i, j;

	if (0 == p[0])
		return(MDOC_MAX);
	if ( ! isalpha((unsigned char)p[0]) && '%' != p[0])
		return(MDOC_MAX);

	if (isalpha((unsigned char)p[1]))
		major = 12 * (tolower((unsigned char)p[1]) - 97);
	else if ('1' == p[1])
		major = 12 * 26;
	else 
		return(MDOC_MAX);

	if (p[2] && p[3])
		return(MDOC_MAX);

	for (j = 0; j < 12; j++) {
		if (UCHAR_MAX == (i = table[major + j]))
			break;
		if (0 == strcmp(p, mdoc_macronames[i]))
			return((enum mdoct)i);
	}

	return(MDOC_MAX);
}