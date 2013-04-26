
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

typedef struct mem_storage{
    unsigned char *base;
    size_t size;
    unsigned char *ptr;
}mem_storage;

static ssize_t
mem_fetch(krb5_storage *sp, void *data, size_t size)
{
    mem_storage *s = (mem_storage*)sp->data;
    if(size > (size_t)(s->base + s->size - s->ptr))
	size = s->base + s->size - s->ptr;
    memmove(data, s->ptr, size);
    sp->seek(sp, size, SEEK_CUR);
    return size;
}

static ssize_t
mem_store(krb5_storage *sp, const void *data, size_t size)
{
    mem_storage *s = (mem_storage*)sp->data;
    if(size > (size_t)(s->base + s->size - s->ptr))
	size = s->base + s->size - s->ptr;
    memmove(s->ptr, data, size);
    sp->seek(sp, size, SEEK_CUR);
    return size;
}

static ssize_t
mem_no_store(krb5_storage *sp, const void *data, size_t size)
{
    return -1;
}

static off_t
mem_seek(krb5_storage *sp, off_t offset, int whence)
{
    mem_storage *s = (mem_storage*)sp->data;
    switch(whence){
    case SEEK_SET:
	if((size_t)offset > s->size)
	    offset = s->size;
	if(offset < 0)
	    offset = 0;
	s->ptr = s->base + offset;
	break;
    case SEEK_CUR:
	return sp->seek(sp, s->ptr - s->base + offset, SEEK_SET);
    case SEEK_END:
	return sp->seek(sp, s->size + offset, SEEK_SET);
    default:
	errno = EINVAL;
	return -1;
    }
    return s->ptr - s->base;
}

static int
mem_trunc(krb5_storage *sp, off_t offset)
{
    mem_storage *s = (mem_storage*)sp->data;
    if((size_t)offset > s->size)
	return ERANGE;
    s->size = offset;
    if ((s->ptr - s->base) > offset)
	s->ptr = s->base + offset;
    return 0;
}

static int
mem_no_trunc(krb5_storage *sp, off_t offset)
{
    return EINVAL;
}

/**
 * Create a fixed size memory storage block
 *
 * @return A krb5_storage on success, or NULL on out of memory error.
 *
 * @ingroup krb5_storage
 *
 * @sa krb5_storage_mem()
 * @sa krb5_storage_from_readonly_mem()
 * @sa krb5_storage_from_data()
 * @sa krb5_storage_from_fd()
 */

KRB5_LIB_FUNCTION krb5_storage * KRB5_LIB_CALL
krb5_storage_from_mem(void *buf, size_t len)
{
    krb5_storage *sp = malloc(sizeof(krb5_storage));
    mem_storage *s;
    if(sp == NULL)
	return NULL;
    s = malloc(sizeof(*s));
    if(s == NULL) {
	free(sp);
	return NULL;
    }
    sp->data = s;
    sp->flags = 0;
    sp->eof_code = HEIM_ERR_EOF;
    s->base = buf;
    s->size = len;
    s->ptr = buf;
    sp->fetch = mem_fetch;
    sp->store = mem_store;
    sp->seek = mem_seek;
    sp->trunc = mem_trunc;
    sp->free = NULL;
    sp->max_alloc = UINT_MAX/8;
    return sp;
}

/**
 * Create a fixed size memory storage block
 *
 * @return A krb5_storage on success, or NULL on out of memory error.
 *
 * @ingroup krb5_storage
 *
 * @sa krb5_storage_mem()
 * @sa krb5_storage_from_mem()
 * @sa krb5_storage_from_readonly_mem()
 * @sa krb5_storage_from_fd()
 */

KRB5_LIB_FUNCTION krb5_storage * KRB5_LIB_CALL
krb5_storage_from_data(krb5_data *data)
{
    return krb5_storage_from_mem(data->data, data->length);
}

/**
 * Create a fixed size memory storage block that is read only
 *
 * @return A krb5_storage on success, or NULL on out of memory error.
 *
 * @ingroup krb5_storage
 *
 * @sa krb5_storage_mem()
 * @sa krb5_storage_from_mem()
 * @sa krb5_storage_from_data()
 * @sa krb5_storage_from_fd()
 */

KRB5_LIB_FUNCTION krb5_storage * KRB5_LIB_CALL
krb5_storage_from_readonly_mem(const void *buf, size_t len)
{
    krb5_storage *sp = malloc(sizeof(krb5_storage));
    mem_storage *s;
    if(sp == NULL)
	return NULL;
    s = malloc(sizeof(*s));
    if(s == NULL) {
	free(sp);
	return NULL;
    }
    sp->data = s;
    sp->flags = 0;
    sp->eof_code = HEIM_ERR_EOF;
    s->base = rk_UNCONST(buf);
    s->size = len;
    s->ptr = rk_UNCONST(buf);
    sp->fetch = mem_fetch;
    sp->store = mem_no_store;
    sp->seek = mem_seek;
    sp->trunc = mem_no_trunc;
    sp->free = NULL;
    sp->max_alloc = UINT_MAX/8;
    return sp;
}