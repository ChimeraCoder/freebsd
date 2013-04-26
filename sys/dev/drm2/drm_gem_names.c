
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/limits.h>
#include <sys/malloc.h>

#include <dev/drm2/drm_gem_names.h>

MALLOC_DEFINE(M_GEM_NAMES, "gem_name", "Hash headers for the gem names");

static void drm_gem_names_delete_name(struct drm_gem_names *names,
    struct drm_gem_name *np);

void
drm_gem_names_init(struct drm_gem_names *names)
{

	names->unr = new_unrhdr(1, INT_MAX, NULL); /* XXXKIB */
	names->names_hash = hashinit(1000 /* XXXKIB */, M_GEM_NAMES,
	    &names->hash_mask);
	mtx_init(&names->lock, "drmnames", NULL, MTX_DEF);
}

void
drm_gem_names_fini(struct drm_gem_names *names)
{
	struct drm_gem_name *np;
	int i;

	mtx_lock(&names->lock);
	for (i = 0; i <= names->hash_mask; i++) {
		while ((np = LIST_FIRST(&names->names_hash[i])) != NULL) {
			drm_gem_names_delete_name(names, np);
			mtx_lock(&names->lock);
		}
	}
	mtx_unlock(&names->lock);
	mtx_destroy(&names->lock);
	hashdestroy(names->names_hash, M_GEM_NAMES, names->hash_mask);
	delete_unrhdr(names->unr);
}

static struct drm_gem_names_head *
gem_name_hash_index(struct drm_gem_names *names, int name)
{

	return (&names->names_hash[name & names->hash_mask]);
}

void *
drm_gem_name_ref(struct drm_gem_names *names, uint32_t name,
    void (*ref)(void *))
{
	struct drm_gem_name *n;

	mtx_lock(&names->lock);
	LIST_FOREACH(n, gem_name_hash_index(names, name), link) {
		if (n->name == name) {
			if (ref != NULL)
				ref(n->ptr);
			mtx_unlock(&names->lock);
			return (n->ptr);
		}
	}
	mtx_unlock(&names->lock);
	return (NULL);
}

struct drm_gem_ptr_match_arg {
	uint32_t res;
	void *ptr;
};

static int
drm_gem_ptr_match(uint32_t name, void *ptr, void *arg)
{
	struct drm_gem_ptr_match_arg *a;

	a = arg;
	if (ptr == a->ptr) {
		a->res = name;
		return (1);
	} else
		return (0);
}

uint32_t
drm_gem_find_name(struct drm_gem_names *names, void *ptr)
{
	struct drm_gem_ptr_match_arg arg;

	arg.res = 0;
	arg.ptr = ptr;
	drm_gem_names_foreach(names, drm_gem_ptr_match, &arg);
	return (arg.res);
}

int
drm_gem_name_create(struct drm_gem_names *names, void *p, uint32_t *name)
{
	struct drm_gem_name *np;

	np = malloc(sizeof(struct drm_gem_name), M_GEM_NAMES, M_WAITOK);
	mtx_lock(&names->lock);
	if (*name != 0) {
		mtx_unlock(&names->lock);
		return (EALREADY);
	}
	np->name = alloc_unr(names->unr);
	if (np->name == -1) {
		mtx_unlock(&names->lock);
		free(np, M_GEM_NAMES);
		return (ENOMEM);
	}
	*name = np->name;
	np->ptr = p;
	LIST_INSERT_HEAD(gem_name_hash_index(names, np->name), np, link);
	mtx_unlock(&names->lock);
	return (0);
}

static void
drm_gem_names_delete_name(struct drm_gem_names *names, struct drm_gem_name *np)
{

	mtx_assert(&names->lock, MA_OWNED);
	LIST_REMOVE(np, link);
	mtx_unlock(&names->lock);
	free_unr(names->unr, np->name);
	free(np, M_GEM_NAMES);
}

void *
drm_gem_names_remove(struct drm_gem_names *names, uint32_t name)
{
	struct drm_gem_name *n;
	void *res;

	mtx_lock(&names->lock);
	LIST_FOREACH(n, gem_name_hash_index(names, name), link) {
		if (n->name == name) {
			res = n->ptr;
			drm_gem_names_delete_name(names, n);
			return (res);
		}
	}
	mtx_unlock(&names->lock);
	return (NULL);
}

void
drm_gem_names_foreach(struct drm_gem_names *names,
    int (*f)(uint32_t, void *, void *), void *arg)
{
	struct drm_gem_name *np;
	struct drm_gem_name marker;
	int i, fres;

	bzero(&marker, sizeof(marker));
	marker.name = -1;
	mtx_lock(&names->lock);
	for (i = 0; i <= names->hash_mask; i++) {
		for (np = LIST_FIRST(&names->names_hash[i]); np != NULL; ) {
			if (np->name == -1) {
				np = LIST_NEXT(np, link);
				continue;
			}
			LIST_INSERT_AFTER(np, &marker, link);
			mtx_unlock(&names->lock);
			fres = f(np->name, np->ptr, arg);
			mtx_lock(&names->lock);
			np = LIST_NEXT(&marker, link);
			LIST_REMOVE(&marker, link);
			if (fres)
				break;
		}
	}
	mtx_unlock(&names->lock);
}