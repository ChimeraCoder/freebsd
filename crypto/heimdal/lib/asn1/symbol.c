
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

#include "gen_locl.h"
#include "lex.h"

static Hashtab *htab;

static int
cmp(void *a, void *b)
{
    Symbol *s1 = (Symbol *) a;
    Symbol *s2 = (Symbol *) b;

    return strcmp(s1->name, s2->name);
}

static unsigned
hash(void *a)
{
    Symbol *s = (Symbol *) a;

    return hashjpw(s->name);
}

void
initsym(void)
{
    htab = hashtabnew(101, cmp, hash);
}


void
output_name(char *s)
{
    char *p;

    for (p = s; *p; ++p)
	if (*p == '-' || *p == '.')
	    *p = '_';
}

Symbol *
addsym(char *name)
{
    Symbol key, *s;

    key.name = name;
    s = (Symbol *) hashtabsearch(htab, (void *) &key);
    if (s == NULL) {
	s = (Symbol *) emalloc(sizeof(*s));
	s->name = name;
	s->gen_name = estrdup(name);
	output_name(s->gen_name);
	s->stype = SUndefined;
	hashtabadd(htab, s);
    }
    return s;
}

static int
checkfunc(void *ptr, void *arg)
{
    Symbol *s = ptr;
    if (s->stype == SUndefined) {
	lex_error_message("%s is still undefined\n", s->name);
	*(int *) arg = 1;
    }
    return 0;
}

int
checkundefined(void)
{
    int f = 0;
    hashtabforeach(htab, checkfunc, &f);
    return f;
}