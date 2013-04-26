
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

#pragma ident	"%Z%%M%	%I%	%E% SMI"

/*
 * Create, manage, and destroy association lists.  alists are arrays with
 * arbitrary index types, and are also commonly known as associative arrays.
 */

#include <stdio.h>
#include <stdlib.h>

#include "alist.h"
#include "memory.h"
#include "hash.h"

#define	ALIST_HASH_SIZE	997

struct alist {
	hash_t *al_elements;
	void (*al_namefree)(void *);
	void (*al_valfree)(void *);
};

typedef struct alist_el {
	void *ale_name;
	void *ale_value;
} alist_el_t;

static int
alist_hash(int nbuckets, void *arg)
{
	alist_el_t *el = arg;
	uintptr_t num = (uintptr_t)el->ale_name;

	return (num % nbuckets);
}

static int
alist_cmp(void *arg1, void *arg2)
{
	alist_el_t *el1 = arg1;
	alist_el_t *el2 = arg2;
	return ((uintptr_t)el1->ale_name != (uintptr_t)el2->ale_name);
}

alist_t *
alist_xnew(int nbuckets, void (*namefree)(void *),
    void (*valfree)(void *), int (*hashfn)(int, void *),
    int (*cmpfn)(void *, void *))
{
	alist_t *alist;

	alist = xcalloc(sizeof (alist_t));
	alist->al_elements = hash_new(nbuckets, hashfn, cmpfn);
	alist->al_namefree = namefree;
	alist->al_valfree = valfree;

	return (alist);
}

alist_t *
alist_new(void (*namefree)(void *), void (*valfree)(void *))
{
	return (alist_xnew(ALIST_HASH_SIZE, namefree, valfree,
	    alist_hash, alist_cmp));
}

static void
alist_free_cb(void *arg1, void *arg2)
{
	alist_el_t *el = arg1;
	alist_t *alist = arg2;
	if (alist->al_namefree)
		alist->al_namefree(el->ale_name);
	if (alist->al_valfree)
		alist->al_valfree(el->ale_name);
	free(el);
}

void
alist_free(alist_t *alist)
{
	hash_free(alist->al_elements, alist_free_cb, alist);
	free(alist);
}

void
alist_add(alist_t *alist, void *name, void *value)
{
	alist_el_t *el;

	el = xmalloc(sizeof (alist_el_t));
	el->ale_name = name;
	el->ale_value = value;
	hash_add(alist->al_elements, el);
}

int
alist_find(alist_t *alist, void *name, void **value)
{
	alist_el_t template, *retx;
	void *ret;

	template.ale_name = name;
	if (!hash_find(alist->al_elements, &template, &ret))
		return (0);

	if (value) {
		retx = ret;
		*value = retx->ale_value;
	}

	return (1);
}

typedef struct alist_iter_data {
	int (*aid_func)(void *, void *, void *);
	void *aid_priv;
} alist_iter_data_t;

static int
alist_iter_cb(void *arg1, void *arg2)
{
	alist_el_t *el = arg1;
	alist_iter_data_t *aid = arg2;
	return (aid->aid_func(el->ale_name, el->ale_value, aid->aid_priv));
}

int
alist_iter(alist_t *alist, int (*func)(void *, void *, void *), void *private)
{
	alist_iter_data_t aid;

	aid.aid_func = func;
	aid.aid_priv = private;

	return (hash_iter(alist->al_elements, alist_iter_cb, &aid));
}

/*
 * Debugging support.  Used to print the contents of an alist.
 */

void
alist_stats(alist_t *alist, int verbose)
{
	printf("Alist statistics\n");
	hash_stats(alist->al_elements, verbose);
}

static int alist_def_print_cb_key_int = 1;
static int alist_def_print_cb_value_int = 1;

static int
alist_def_print_cb(void *key, void *value)
{
	printf("Key: ");
	if (alist_def_print_cb_key_int == 1)
		printf("%5lu ", (ulong_t)key);
	else
		printf("%s\n", (char *)key);

	printf("Value: ");
	if (alist_def_print_cb_value_int == 1)
		printf("%5lu\n", (ulong_t)value);
	else
		printf("%s\n", (char *)key);

	return (1);
}

static int
alist_dump_cb(void *node, void *private)
{
	int (*printer)(void *, void *) = private;
	alist_el_t *el = node;

	printer(el->ale_name, el->ale_value);

	return (1);
}

int
alist_dump(alist_t *alist, int (*printer)(void *, void *))
{
	if (!printer)
		printer = alist_def_print_cb;

	return (hash_iter(alist->al_elements, alist_dump_cb, (void *)printer));
}