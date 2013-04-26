
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
/* $Id: symtab.c,v 1.10 2012/05/26 15:16:12 tom Exp $ */
#include "defs.h"

/* TABLE_SIZE is the number of entries in the symbol table. */
/* TABLE_SIZE must be a power of two.			    */

#define	TABLE_SIZE 1024

static bucket **symbol_table = 0;
bucket *first_symbol;
bucket *last_symbol;

static int
hash(const char *name)
{
    const char *s;
    int c, k;

    assert(name && *name);
    s = name;
    k = *s;
    while ((c = *++s) != 0)
	k = (31 * k + c) & (TABLE_SIZE - 1);

    return (k);
}

bucket *
make_bucket(const char *name)
{
    bucket *bp;

    assert(name != 0);

    bp = TMALLOC(bucket, 1);
    NO_SPACE(bp);

    bp->link = 0;
    bp->next = 0;

    bp->name = TMALLOC(char, strlen(name) + 1);
    NO_SPACE(bp->name);

    bp->tag = 0;
    bp->value = UNDEFINED;
    bp->index = 0;
    bp->prec = 0;
    bp->class = UNKNOWN;
    bp->assoc = TOKEN;
    strcpy(bp->name, name);

    return (bp);
}

bucket *
lookup(const char *name)
{
    bucket *bp, **bpp;

    bpp = symbol_table + hash(name);
    bp = *bpp;

    while (bp)
    {
	if (strcmp(name, bp->name) == 0)
	    return (bp);
	bpp = &bp->link;
	bp = *bpp;
    }

    *bpp = bp = make_bucket(name);
    last_symbol->next = bp;
    last_symbol = bp;

    return (bp);
}

void
create_symbol_table(void)
{
    int i;
    bucket *bp;

    symbol_table = TMALLOC(bucket *, TABLE_SIZE);
    NO_SPACE(symbol_table);

    for (i = 0; i < TABLE_SIZE; i++)
	symbol_table[i] = 0;

    bp = make_bucket("error");
    bp->index = 1;
    bp->class = TERM;

    first_symbol = bp;
    last_symbol = bp;
    symbol_table[hash("error")] = bp;
}

void
free_symbol_table(void)
{
    FREE(symbol_table);
    symbol_table = 0;
}

void
free_symbols(void)
{
    bucket *p, *q;

    for (p = first_symbol; p; p = q)
    {
	q = p->next;
	FREE(p);
    }
}