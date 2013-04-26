
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

/**
 * \file
 * Regional allocator. Allocates small portions of of larger chunks.
 */

#include "config.h"
#include "util/log.h"
#include "util/regional.h"

#ifdef ALIGNMENT
#  undef ALIGNMENT
#endif
/** increase size until it fits alignment of s bytes */
#define ALIGN_UP(x, s)     (((x) + s - 1) & (~(s - 1)))
/** what size to align on; make sure a char* fits in it. */
#define ALIGNMENT          (sizeof(uint64_t))

/** Default reasonable size for chunks */
#define REGIONAL_CHUNK_SIZE         8192
#ifdef UNBOUND_ALLOC_NONREGIONAL
/** All objects allocated outside of chunks, for debug */
#define REGIONAL_LARGE_OBJECT_SIZE  0
#else
/** Default size for large objects - allocated outside of chunks. */
#define REGIONAL_LARGE_OBJECT_SIZE  2048
#endif

struct regional* 
regional_create(void)
{
	return regional_create_custom(REGIONAL_CHUNK_SIZE);
}

/** init regional struct with first block */
static void
regional_init(struct regional* r)
{
	size_t a = ALIGN_UP(sizeof(struct regional), ALIGNMENT);
	r->data = (char*)r + a;
	r->available = r->first_size - a;
	r->next = NULL;
	r->large_list = NULL;
	r->total_large = 0;
}

struct regional* 
regional_create_custom(size_t size)
{
	struct regional* r = (struct regional*)malloc(size);
	log_assert(sizeof(struct regional) <= size);
	if(!r) return NULL;
	r->first_size = size;
	regional_init(r);
	return r;
}

void 
regional_free_all(struct regional *r)
{
	char* p = r->next, *np;
	while(p) {
		np = *(char**)p;
		free(p);
		p = np;
	}
	p = r->large_list;
	while(p) {
		np = *(char**)p;
		free(p);
		p = np;
	}
	regional_init(r);
}

void 
regional_destroy(struct regional *r)
{
	if(!r) return;
	regional_free_all(r);
	free(r);
}

void *
regional_alloc(struct regional *r, size_t size)
{
	size_t a = ALIGN_UP(size, ALIGNMENT);
	void *s;
	/* large objects */
	if(a > REGIONAL_LARGE_OBJECT_SIZE) {
		s = malloc(ALIGNMENT + size);
		if(!s) return NULL;
		r->total_large += ALIGNMENT+size;
		*(char**)s = r->large_list;
		r->large_list = (char*)s;
		return (char*)s+ALIGNMENT;
	}
	/* create a new chunk */
	if(a > r->available) {
		s = malloc(REGIONAL_CHUNK_SIZE);
		if(!s) return NULL;
		*(char**)s = r->next;
		r->next = (char*)s;
		r->data = (char*)s + ALIGNMENT;
		r->available = REGIONAL_CHUNK_SIZE - ALIGNMENT;
	}
	/* put in this chunk */
	r->available -= a;
	s = r->data;
	r->data += a;
	return s;
}

void *
regional_alloc_init(struct regional* r, const void *init, size_t size)
{
	void *s = regional_alloc(r, size);
	if(!s) return NULL;
	memcpy(s, init, size);
	return s;
}

void *
regional_alloc_zero(struct regional *r, size_t size)
{
	void *s = regional_alloc(r, size);
	if(!s) return NULL;
	memset(s, 0, size);
	return s;
}

char *
regional_strdup(struct regional *r, const char *string)
{
	return (char*)regional_alloc_init(r, string, strlen(string)+1);
}

/**
 * reasonably slow, but stats and get_mem are not supposed to be fast
 * count the number of chunks in use
 */
static size_t
count_chunks(struct regional* r)
{
	size_t c = 1;
	char* p = r->next;
	while(p) {
		c++;
		p = *(char**)p;
	}
	return c;
}

/**
 * also reasonably slow, counts the number of large objects
 */
static size_t
count_large(struct regional* r)
{
	size_t c = 0;
	char* p = r->large_list;
	while(p) {
		c++;
		p = *(char**)p;
	}
	return c;
}

void 
regional_log_stats(struct regional *r)
{
	/* some basic assertions put here (non time critical code) */
	log_assert(ALIGNMENT >= sizeof(char*));
	log_assert(REGIONAL_CHUNK_SIZE > ALIGNMENT);
	log_assert(REGIONAL_CHUNK_SIZE-ALIGNMENT > REGIONAL_LARGE_OBJECT_SIZE);
	log_assert(REGIONAL_CHUNK_SIZE >= sizeof(struct regional));
	/* debug print */
	log_info("regional %u chunks, %u large",
		(unsigned)count_chunks(r), (unsigned)count_large(r));
}

size_t 
regional_get_mem(struct regional* r)
{
	return r->first_size + (count_chunks(r)-1)*REGIONAL_CHUNK_SIZE 
		+ r->total_large;
}