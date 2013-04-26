
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

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "mandoc.h"
#include "libmandoc.h"

#define	PRINT_HI	 126
#define	PRINT_LO	 32

struct	ln {
	struct ln	 *next;
	const char	 *code;
	const char	 *ascii;
	int		  unicode;
};

#define	LINES_MAX	  328

#define CHAR(in, ch, code) \
	{ NULL, (in), (ch), (code) },

#define	CHAR_TBL_START	  static struct ln lines[LINES_MAX] = {
#define	CHAR_TBL_END	  };

#include "chars.in"

struct	mchars {
	struct ln	**htab;
};

static	const struct ln	 *find(const struct mchars *, 
				const char *, size_t);

void
mchars_free(struct mchars *arg)
{

	free(arg->htab);
	free(arg);
}

struct mchars *
mchars_alloc(void)
{
	struct mchars	 *tab;
	struct ln	**htab;
	struct ln	 *pp;
	int		  i, hash;

	/*
	 * Constructs a very basic chaining hashtable.  The hash routine
	 * is simply the integral value of the first character.
	 * Subsequent entries are chained in the order they're processed.
	 */

	tab = mandoc_malloc(sizeof(struct mchars));
	htab = mandoc_calloc(PRINT_HI - PRINT_LO + 1, sizeof(struct ln **));

	for (i = 0; i < LINES_MAX; i++) {
		hash = (int)lines[i].code[0] - PRINT_LO;

		if (NULL == (pp = htab[hash])) {
			htab[hash] = &lines[i];
			continue;
		}

		for ( ; pp->next; pp = pp->next)
			/* Scan ahead. */ ;
		pp->next = &lines[i];
	}

	tab->htab = htab;
	return(tab);
}

int
mchars_spec2cp(const struct mchars *arg, const char *p, size_t sz)
{
	const struct ln	*ln;

	ln = find(arg, p, sz);
	if (NULL == ln)
		return(-1);
	return(ln->unicode);
}

char
mchars_num2char(const char *p, size_t sz)
{
	int		  i;

	if ((i = mandoc_strntoi(p, sz, 10)) < 0)
		return('\0');
	return(i > 0 && i < 256 && isprint(i) ? 
			/* LINTED */ i : '\0');
}

int
mchars_num2uc(const char *p, size_t sz)
{
	int               i;

	if ((i = mandoc_strntoi(p, sz, 16)) < 0)
		return('\0');
	/* FIXME: make sure we're not in a bogus range. */
	return(i > 0x80 && i <= 0x10FFFF ? i : '\0');
}

const char *
mchars_spec2str(const struct mchars *arg, 
		const char *p, size_t sz, size_t *rsz)
{
	const struct ln	*ln;

	ln = find(arg, p, sz);
	if (NULL == ln) {
		*rsz = 1;
		return(NULL);
	}

	*rsz = strlen(ln->ascii);
	return(ln->ascii);
}

static const struct ln *
find(const struct mchars *tab, const char *p, size_t sz)
{
	const struct ln	 *pp;
	int		  hash;

	assert(p);

	if (0 == sz || p[0] < PRINT_LO || p[0] > PRINT_HI)
		return(NULL);

	hash = (int)p[0] - PRINT_LO;

	for (pp = tab->htab[hash]; pp; pp = pp->next)
		if (0 == strncmp(pp->code, p, sz) && 
				'\0' == pp->code[(int)sz])
			return(pp);

	return(NULL);
}