
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

#include "kdc_locl.h"

/*
 *
 */

void
krb5_kdc_update_time(struct timeval *tv)
{
    if (tv == NULL)
	gettimeofday(&_kdc_now, NULL);
    else
	_kdc_now = *tv;
}

static krb5_error_code
kdc_as_req(krb5_context context,
	   krb5_kdc_configuration *config,
	   krb5_data *req_buffer,
	   krb5_data *reply,
	   const char *from,
	   struct sockaddr *addr,
	   int datagram_reply,
	   int *claim)
{
    krb5_error_code ret;
    KDC_REQ req;
    size_t len;

    ret = decode_AS_REQ(req_buffer->data, req_buffer->length, &req, &len);
    if (ret)
	return ret;

    *claim = 1;

    ret = _kdc_as_rep(context, config, &req, req_buffer,
		      reply, from, addr, datagram_reply);
    free_AS_REQ(&req);
    return ret;
}


static krb5_error_code
kdc_tgs_req(krb5_context context,
	    krb5_kdc_configuration *config,
	    krb5_data *req_buffer,
	    krb5_data *reply,
	    const char *from,
	    struct sockaddr *addr,
	    int datagram_reply,
	    int *claim)
{
    krb5_error_code ret;
    KDC_REQ req;
    size_t len;

    ret = decode_TGS_REQ(req_buffer->data, req_buffer->length, &req, &len);
    if (ret)
	return ret;

    *claim = 1;

    ret = _kdc_tgs_rep(context, config, &req, reply,
		       from, addr, datagram_reply);
    free_TGS_REQ(&req);
    return ret;
}

#ifdef DIGEST

static krb5_error_code
kdc_digest(krb5_context context,
	   krb5_kdc_configuration *config,
	   krb5_data *req_buffer,
	   krb5_data *reply,
	   const char *from,
	   struct sockaddr *addr,
	   int datagram_reply,
	   int *claim)
{
    DigestREQ digestreq;
    krb5_error_code ret;
    size_t len;

    ret = decode_DigestREQ(req_buffer->data, req_buffer->length,
			   &digestreq, &len);
    if (ret)
	return ret;

    *claim = 1;

    ret = _kdc_do_digest(context, config, &digestreq, reply, from, addr);
    free_DigestREQ(&digestreq);
    return ret;
}

#endif

#ifdef KX509

static krb5_error_code
kdc_kx509(krb5_context context,
	  krb5_kdc_configuration *config,
	  krb5_data *req_buffer,
	  krb5_data *reply,
	  const char *from,
	  struct sockaddr *addr,
	  int datagram_reply,
	  int *claim)
{
    Kx509Request kx509req;
    krb5_error_code ret;
    size_t len;

    ret = _kdc_try_kx509_request(req_buffer->data, req_buffer->length,
				 &kx509req, &len);
    if (ret)
	return ret;

    *claim = 1;

    ret = _kdc_do_kx509(context, config, &kx509req, reply, from, addr);
    free_Kx509Request(&kx509req);
    return ret;
}

#endif


static struct krb5_kdc_service services[] =  {
    { KS_KRB5,		kdc_as_req },
    { KS_KRB5,		kdc_tgs_req },
#ifdef DIGEST
    { 0,		kdc_digest },
#endif
#ifdef KX509
    { 0,		kdc_kx509 },
#endif
    { 0, NULL }
};

/*
 * handle the request in `buf, len', from `addr' (or `from' as a string),
 * sending a reply in `reply'.
 */

int
krb5_kdc_process_request(krb5_context context,
			 krb5_kdc_configuration *config,
			 unsigned char *buf,
			 size_t len,
			 krb5_data *reply,
			 krb5_boolean *prependlength,
			 const char *from,
			 struct sockaddr *addr,
			 int datagram_reply)
{
    krb5_error_code ret;
    unsigned int i;
    krb5_data req_buffer;
    int claim = 0;

    req_buffer.data = buf;
    req_buffer.length = len;

    for (i = 0; services[i].process != NULL; i++) {
	ret = (*services[i].process)(context, config, &req_buffer,
				     reply, from, addr, datagram_reply,
				     &claim);
	if (claim) {
	    if (services[i].flags & KS_NO_LENGTH)
		*prependlength = 0;
	    return ret;
	}
    }

    return -1;
}

/*
 * handle the request in `buf, len', from `addr' (or `from' as a string),
 * sending a reply in `reply'.
 *
 * This only processes krb5 requests
 */

int
krb5_kdc_process_krb5_request(krb5_context context,
			      krb5_kdc_configuration *config,
			      unsigned char *buf,
			      size_t len,
			      krb5_data *reply,
			      const char *from,
			      struct sockaddr *addr,
			      int datagram_reply)
{
    krb5_error_code ret;
    unsigned int i;
    krb5_data req_buffer;
    int claim = 0;

    req_buffer.data = buf;
    req_buffer.length = len;

    for (i = 0; services[i].process != NULL; i++) {
	if ((services[i].flags & KS_KRB5) == 0)
	    continue;
	ret = (*services[i].process)(context, config, &req_buffer,
				     reply, from, addr, datagram_reply,
				     &claim);
	if (claim)
	    return ret;
    }

    return -1;
}

/*
 *
 */

int
krb5_kdc_save_request(krb5_context context,
		      const char *fn,
		      const unsigned char *buf,
		      size_t len,
		      const krb5_data *reply,
		      const struct sockaddr *sa)
{
    krb5_storage *sp;
    krb5_address a;
    int fd, ret;
    uint32_t t;
    krb5_data d;

    memset(&a, 0, sizeof(a));

    d.data = rk_UNCONST(buf);
    d.length = len;
    t = _kdc_now.tv_sec;

    fd = open(fn, O_WRONLY|O_CREAT|O_APPEND, 0600);
    if (fd < 0) {
	int saved_errno = errno;
	krb5_set_error_message(context, saved_errno, "Failed to open: %s", fn);
	return saved_errno;
    }

    sp = krb5_storage_from_fd(fd);
    close(fd);
    if (sp == NULL) {
	krb5_set_error_message(context, ENOMEM, "Storage failed to open fd");
	return ENOMEM;
    }

    ret = krb5_sockaddr2address(context, sa, &a);
    if (ret)
	goto out;

    krb5_store_uint32(sp, 1);
    krb5_store_uint32(sp, t);
    krb5_store_address(sp, a);
    krb5_store_data(sp, d);
    {
	Der_class cl;
	Der_type ty;
	unsigned int tag;
	ret = der_get_tag (reply->data, reply->length,
			   &cl, &ty, &tag, NULL);
	if (ret) {
	    krb5_store_uint32(sp, 0xffffffff);
	    krb5_store_uint32(sp, 0xffffffff);
	} else {
	    krb5_store_uint32(sp, MAKE_TAG(cl, ty, 0));
	    krb5_store_uint32(sp, tag);
	}
    }

    krb5_free_address(context, &a);
out:
    krb5_storage_free(sp);

    return 0;
}