
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
#include <string.h>

#include "man.h"
#include "mandoc.h"
#include "libman.h"

#define	HASH_DEPTH	 6

#define	HASH_ROW(x) do { \
		if (isupper((unsigned char)(x))) \
			(x) -= 65; \
		else \
			(x) -= 97; \
		(x) *= HASH_DEPTH; \
	} while (/* CONSTCOND */ 0)

/*
 * Lookup table is indexed first by lower-case first letter (plus one
 * for the period, which is stored in the last row), then by lower or
 * uppercase second letter.  Buckets correspond to the index of the
 * macro (the integer value of the enum stored as a char to save a bit
 * of space).
 */
static	unsigned char	 table[26 * HASH_DEPTH];

/*
 * XXX - this hash has global scope, so if intended for use as a library
 * with multiple callers, it will need re-invocation protection.
 */
void
man_hash_init(void)
{
	int		 i, j, x;

	memset(table, UCHAR_MAX, sizeof(table));

	assert(/* LINTED */ 
			MAN_MAX < UCHAR_MAX);

	for (i = 0; i < (int)MAN_MAX; i++) {
		x = man_macronames[i][0];

		assert(isalpha((unsigned char)x));

		HASH_ROW(x);

		for (j = 0; j < HASH_DEPTH; j++)
			if (UCHAR_MAX == table[x + j]) {
				table[x + j] = (unsigned char)i;
				break;
			}

		assert(j < HASH_DEPTH);
	}
}


enum mant
man_hash_find(const char *tmp)
{
	int		 x, y, i;
	enum mant	 tok;

	if ('\0' == (x = tmp[0]))
		return(MAN_MAX);
	if ( ! (isalpha((unsigned char)x)))
		return(MAN_MAX);

	HASH_ROW(x);

	for (i = 0; i < HASH_DEPTH; i++) {
		if (UCHAR_MAX == (y = table[x + i]))
			return(MAN_MAX);

		tok = (enum mant)y;
		if (0 == strcmp(tmp, man_macronames[tok]))
			return(tok);
	}

	return(MAN_MAX);
}