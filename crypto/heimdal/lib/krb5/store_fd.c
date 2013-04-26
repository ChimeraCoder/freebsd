
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

typedef struct fd_storage {
    int fd;
} fd_storage;

#define FD(S) (((fd_storage*)(S)->data)->fd)

static ssize_t
fd_fetch(krb5_storage * sp, void *data, size_t size)
{
    return net_read(FD(sp), data, size);
}

static ssize_t
fd_store(krb5_storage * sp, const void *data, size_t size)
{
    return net_write(FD(sp), data, size);
}

static off_t
fd_seek(krb5_storage * sp, off_t offset, int whence)
{
    return lseek(FD(sp), offset, whence);
}

static int
fd_trunc(krb5_storage * sp, off_t offset)
{
    if (ftruncate(FD(sp), offset) == -1)
	return errno;
    return 0;
}

static void
fd_free(krb5_storage * sp)
{
    close(FD(sp));
}

/**
 *
 *
 * @return A krb5_storage on success, or NULL on out of memory error.
 *
 * @ingroup krb5_storage
 *
 * @sa krb5_storage_emem()
 * @sa krb5_storage_from_mem()
 * @sa krb5_storage_from_readonly_mem()
 * @sa krb5_storage_from_data()
 */

KRB5_LIB_FUNCTION krb5_storage * KRB5_LIB_CALL
krb5_storage_from_fd(krb5_socket_t fd_in)
{
    krb5_storage *sp;
    int fd;

#ifdef SOCKET_IS_NOT_AN_FD
#ifdef _MSC_VER
    if (_get_osfhandle(fd_in) != -1) {
	fd = dup(fd_in);
    } else {
	fd = _open_osfhandle(fd_in, 0);
    }
#else
#error Dont know how to deal with fd that may or may not be a socket.
#endif
#else  /* SOCKET_IS_NOT_AN_FD */
    fd = dup(fd_in);
#endif

    if (fd < 0)
	return NULL;

    sp = malloc(sizeof(krb5_storage));
    if (sp == NULL) {
	close(fd);
	return NULL;
    }

    sp->data = malloc(sizeof(fd_storage));
    if (sp->data == NULL) {
	close(fd);
	free(sp);
	return NULL;
    }
    sp->flags = 0;
    sp->eof_code = HEIM_ERR_EOF;
    FD(sp) = fd;
    sp->fetch = fd_fetch;
    sp->store = fd_store;
    sp->seek = fd_seek;
    sp->trunc = fd_trunc;
    sp->free = fd_free;
    sp->max_alloc = UINT_MAX/8;
    return sp;
}