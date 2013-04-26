
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

#include "krb5_locl.h"
#include "store-int.h"

typedef struct emem_storage{
    unsigned char *base;
    size_t size;
    size_t len;
    unsigned char *ptr;
}emem_storage;

static ssize_t
emem_fetch(krb5_storage *sp, void *data, size_t size)
{
    emem_storage *s = (emem_storage*)sp->data;
    if((size_t)(s->base + s->len - s->ptr) < size)
	size = s->base + s->len - s->ptr;
    memmove(data, s->ptr, size);
    sp->seek(sp, size, SEEK_CUR);
    return size;
}

static ssize_t
emem_store(krb5_storage *sp, const void *data, size_t size)
{
    emem_storage *s = (emem_storage*)sp->data;
    if(size > (size_t)(s->base + s->size - s->ptr)){
	void *base;
	size_t sz, off;
	off = s->ptr - s->base;
	sz = off + size;
	if (sz < 4096)
	    sz *= 2;
	base = realloc(s->base, sz);
	if(base == NULL)
	    return -1;
	s->size = sz;
	s->base = base;
	s->ptr = (unsigned char*)base + off;
    }
    memmove(s->ptr, data, size);
    sp->seek(sp, size, SEEK_CUR);
    return size;
}

static off_t
emem_seek(krb5_storage *sp, off_t offset, int whence)
{
    emem_storage *s = (emem_storage*)sp->data;
    switch(whence){
    case SEEK_SET:
	if((size_t)offset > s->size)
	    offset = s->size;
	if(offset < 0)
	    offset = 0;
	s->ptr = s->base + offset;
	if((size_t)offset > s->len)
	    s->len = offset;
	break;
    case SEEK_CUR:
	sp->seek(sp,s->ptr - s->base + offset, SEEK_SET);
	break;
    case SEEK_END:
	sp->seek(sp, s->len + offset, SEEK_SET);
	break;
    default:
	errno = EINVAL;
	return -1;
    }
    return s->ptr - s->base;
}

static int
emem_trunc(krb5_storage *sp, off_t offset)
{
    emem_storage *s = (emem_storage*)sp->data;
    /*
     * If offset is larget then current size, or current size is
     * shrunk more then half of the current size, adjust buffer.
     */
    if (offset == 0) {
	free(s->base);
	s->size = 0;
	s->base = NULL;
	s->ptr = NULL;
    } else if ((size_t)offset > s->size || (s->size / 2) > (size_t)offset) {
	void *base;
	size_t off;
	off = s->ptr - s->base;
	base = realloc(s->base, offset);
	if(base == NULL)
	    return ENOMEM;
	if ((size_t)offset > s->size)
	    memset((char *)base + s->size, 0, offset - s->size);
	s->size = offset;
	s->base = base;
	s->ptr = (unsigned char *)base + off;
    }
    s->len = offset;
    if ((s->ptr - s->base) > offset)
	s->ptr = s->base + offset;
    return 0;
}


static void
emem_free(krb5_storage *sp)
{
    emem_storage *s = sp->data;
    memset(s->base, 0, s->len);
    free(s->base);
}

/**
 * Create a elastic (allocating) memory storage backend. Memory is
 * allocated on demand. Free returned krb5_storage with
 * krb5_storage_free().
 *
 * @return A krb5_storage on success, or NULL on out of memory error.
 *
 * @ingroup krb5_storage
 *
 * @sa krb5_storage_from_mem()
 * @sa krb5_storage_from_readonly_mem()
 * @sa krb5_storage_from_fd()
 * @sa krb5_storage_from_data()
 */

KRB5_LIB_FUNCTION krb5_storage * KRB5_LIB_CALL
krb5_storage_emem(void)
{
    krb5_storage *sp;
    emem_storage *s;

    sp = malloc(sizeof(krb5_storage));
    if (sp == NULL)
	return NULL;

    s = malloc(sizeof(*s));
    if (s == NULL) {
	free(sp);
	return NULL;
    }
    sp->data = s;
    sp->flags = 0;
    sp->eof_code = HEIM_ERR_EOF;
    s->size = 1024;
    s->base = malloc(s->size);
    if (s->base == NULL) {
	free(sp);
	free(s);
	return NULL;
    }
    s->len = 0;
    s->ptr = s->base;
    sp->fetch = emem_fetch;
    sp->store = emem_store;
    sp->seek = emem_seek;
    sp->trunc = emem_trunc;
    sp->free = emem_free;
    sp->max_alloc = UINT_MAX/8;
    return sp;
}