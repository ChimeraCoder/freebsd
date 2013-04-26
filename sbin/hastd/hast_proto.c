
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

#include <sys/endian.h>

#include <errno.h>
#include <strings.h>

#include <hast.h>
#include <ebuf.h>
#include <nv.h>
#include <pjdlog.h>
#include <proto.h>

#ifdef HAVE_CRYPTO
#include "hast_checksum.h"
#endif
#include "hast_compression.h"
#include "hast_proto.h"

struct hast_main_header {
	/* Protocol version. */
	uint8_t		version;
	/* Size of nv headers. */
	uint32_t	size;
} __packed;

typedef int hps_send_t(const struct hast_resource *, struct nv *nv, void **,
    size_t *, bool *);
typedef int hps_recv_t(const struct hast_resource *, struct nv *nv, void **,
    size_t *, bool *);

struct hast_pipe_stage {
	const char	*hps_name;
	hps_send_t	*hps_send;
	hps_recv_t	*hps_recv;
};

static struct hast_pipe_stage pipeline[] = {
	{ "compression", compression_send, compression_recv },
#ifdef HAVE_CRYPTO
	{ "checksum", checksum_send, checksum_recv }
#endif
};

/*
 * Send the given nv structure via conn.
 * We keep headers in nv structure and pass data in separate argument.
 * There can be no data at all (data is NULL then).
 */
int
hast_proto_send(const struct hast_resource *res, struct proto_conn *conn,
    struct nv *nv, const void *data, size_t size)
{
	struct hast_main_header hdr;
	struct ebuf *eb;
	bool freedata;
	void *dptr, *hptr;
	size_t hsize;
	int ret;

	dptr = (void *)(uintptr_t)data;
	freedata = false;
	ret = -1;

	if (data != NULL) {
		unsigned int ii;

		for (ii = 0; ii < sizeof(pipeline) / sizeof(pipeline[0]);
		    ii++) {
			(void)pipeline[ii].hps_send(res, nv, &dptr, &size,
			    &freedata);
		}
		nv_add_uint32(nv, size, "size");
		if (nv_error(nv) != 0) {
			errno = nv_error(nv);
			goto end;
		}
	}

	eb = nv_hton(nv);
	if (eb == NULL)
		goto end;

	hdr.version = res != NULL ? res->hr_version : HAST_PROTO_VERSION;
	hdr.size = htole32((uint32_t)ebuf_size(eb));
	if (ebuf_add_head(eb, &hdr, sizeof(hdr)) == -1)
		goto end;

	hptr = ebuf_data(eb, &hsize);
	if (proto_send(conn, hptr, hsize) == -1)
		goto end;
	if (data != NULL && proto_send(conn, dptr, size) == -1)
		goto end;

	ret = 0;
end:
	if (freedata)
		free(dptr);
	return (ret);
}

int
hast_proto_recv_hdr(const struct proto_conn *conn, struct nv **nvp)
{
	struct hast_main_header hdr;
	struct nv *nv;
	struct ebuf *eb;
	void *hptr;

	eb = NULL;
	nv = NULL;

	if (proto_recv(conn, &hdr, sizeof(hdr)) == -1)
		goto fail;

	if (hdr.version > HAST_PROTO_VERSION) {
		errno = ERPCMISMATCH;
		goto fail;
	}

	hdr.size = le32toh(hdr.size);

	eb = ebuf_alloc(hdr.size);
	if (eb == NULL)
		goto fail;
	if (ebuf_add_tail(eb, NULL, hdr.size) == -1)
		goto fail;
	hptr = ebuf_data(eb, NULL);
	PJDLOG_ASSERT(hptr != NULL);
	if (proto_recv(conn, hptr, hdr.size) == -1)
		goto fail;
	nv = nv_ntoh(eb);
	if (nv == NULL)
		goto fail;

	*nvp = nv;
	return (0);
fail:
	if (eb != NULL)
		ebuf_free(eb);
	return (-1);
}

int
hast_proto_recv_data(const struct hast_resource *res, struct proto_conn *conn,
    struct nv *nv, void *data, size_t size)
{
	unsigned int ii;
	bool freedata;
	size_t dsize;
	void *dptr;
	int ret;

	PJDLOG_ASSERT(data != NULL);
	PJDLOG_ASSERT(size > 0);

	ret = -1;
	freedata = false;
	dptr = data;

	dsize = nv_get_uint32(nv, "size");
	if (dsize > size) {
		errno = EINVAL;
		goto end;
	} else if (dsize == 0) {
		(void)nv_set_error(nv, 0);
	} else {
		if (proto_recv(conn, data, dsize) == -1)
			goto end;
		for (ii = sizeof(pipeline) / sizeof(pipeline[0]); ii > 0;
		    ii--) {
			ret = pipeline[ii - 1].hps_recv(res, nv, &dptr,
			    &dsize, &freedata);
			if (ret == -1)
				goto end;
		}
		ret = -1;
		if (dsize > size) {
			errno = EINVAL;
			goto end;
		}
		if (dptr != data)
			bcopy(dptr, data, dsize);
	}

	ret = 0;
end:
	if (freedata)
		free(dptr);
	return (ret);
}