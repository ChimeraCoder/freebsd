
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

/*
 *  glue routine for _gsskrb5_inquire_sec_context_by_oid
 */

#include "gsskrb5_locl.h"

static OM_uint32
get_bool(OM_uint32 *minor_status,
	 const gss_buffer_t value,
	 int *flag)
{
    if (value->value == NULL || value->length != 1) {
	*minor_status = EINVAL;
	return GSS_S_FAILURE;
    }
    *flag = *((const char *)value->value) != 0;
    return GSS_S_COMPLETE;
}

static OM_uint32
get_string(OM_uint32 *minor_status,
	   const gss_buffer_t value,
	   char **str)
{
    if (value == NULL || value->length == 0) {
	*str = NULL;
    } else {
	*str = malloc(value->length + 1);
	if (*str == NULL) {
	    *minor_status = 0;
	    return GSS_S_UNAVAILABLE;
	}
	memcpy(*str, value->value, value->length);
	(*str)[value->length] = '\0';
    }
    return GSS_S_COMPLETE;
}

static OM_uint32
get_int32(OM_uint32 *minor_status,
	  const gss_buffer_t value,
	  OM_uint32 *ret)
{
    *minor_status = 0;
    if (value == NULL || value->length == 0)
	*ret = 0;
    else if (value->length == sizeof(*ret))
	memcpy(ret, value->value, sizeof(*ret));
    else
	return GSS_S_UNAVAILABLE;

    return GSS_S_COMPLETE;
}

static OM_uint32
set_int32(OM_uint32 *minor_status,
	  const gss_buffer_t value,
	  OM_uint32 set)
{
    *minor_status = 0;
    if (value->length == sizeof(set))
	memcpy(value->value, &set, sizeof(set));
    else
	return GSS_S_UNAVAILABLE;

    return GSS_S_COMPLETE;
}

OM_uint32 GSSAPI_CALLCONV
_gsskrb5_set_sec_context_option
           (OM_uint32 *minor_status,
            gss_ctx_id_t *context_handle,
            const gss_OID desired_object,
            const gss_buffer_t value)
{
    krb5_context context;
    OM_uint32 maj_stat;

    GSSAPI_KRB5_INIT (&context);

    if (value == GSS_C_NO_BUFFER) {
	*minor_status = EINVAL;
	return GSS_S_FAILURE;
    }

    if (gss_oid_equal(desired_object, GSS_KRB5_COMPAT_DES3_MIC_X)) {
	gsskrb5_ctx ctx;
	int flag;

	if (*context_handle == GSS_C_NO_CONTEXT) {
	    *minor_status = EINVAL;
	    return GSS_S_NO_CONTEXT;
	}

	maj_stat = get_bool(minor_status, value, &flag);
	if (maj_stat != GSS_S_COMPLETE)
	    return maj_stat;

	ctx = (gsskrb5_ctx)*context_handle;
	HEIMDAL_MUTEX_lock(&ctx->ctx_id_mutex);
	if (flag)
	    ctx->more_flags |= COMPAT_OLD_DES3;
	else
	    ctx->more_flags &= ~COMPAT_OLD_DES3;
	ctx->more_flags |= COMPAT_OLD_DES3_SELECTED;
	HEIMDAL_MUTEX_unlock(&ctx->ctx_id_mutex);
	return GSS_S_COMPLETE;
    } else if (gss_oid_equal(desired_object, GSS_KRB5_SET_DNS_CANONICALIZE_X)) {
	int flag;

	maj_stat = get_bool(minor_status, value, &flag);
	if (maj_stat != GSS_S_COMPLETE)
	    return maj_stat;

	krb5_set_dns_canonicalize_hostname(context, flag);
	return GSS_S_COMPLETE;

    } else if (gss_oid_equal(desired_object, GSS_KRB5_REGISTER_ACCEPTOR_IDENTITY_X)) {
	char *str;

	maj_stat = get_string(minor_status, value, &str);
	if (maj_stat != GSS_S_COMPLETE)
	    return maj_stat;

	maj_stat = _gsskrb5_register_acceptor_identity(minor_status, str);
	free(str);

	return maj_stat;

    } else if (gss_oid_equal(desired_object, GSS_KRB5_SET_DEFAULT_REALM_X)) {
	char *str;

	maj_stat = get_string(minor_status, value, &str);
	if (maj_stat != GSS_S_COMPLETE)
	    return maj_stat;
	if (str == NULL) {
	    *minor_status = 0;
	    return GSS_S_CALL_INACCESSIBLE_READ;
	}

	krb5_set_default_realm(context, str);
	free(str);

	*minor_status = 0;
	return GSS_S_COMPLETE;

    } else if (gss_oid_equal(desired_object, GSS_KRB5_SEND_TO_KDC_X)) {

	if (value == NULL || value->length == 0) {
	    krb5_set_send_to_kdc_func(context, NULL, NULL);
	} else {
	    struct gsskrb5_send_to_kdc c;

	    if (value->length != sizeof(c)) {
		*minor_status = EINVAL;
		return GSS_S_FAILURE;
	    }
	    memcpy(&c, value->value, sizeof(c));
	    krb5_set_send_to_kdc_func(context,
				      (krb5_send_to_kdc_func)c.func,
				      c.ptr);
	}

	*minor_status = 0;
	return GSS_S_COMPLETE;
    } else if (gss_oid_equal(desired_object, GSS_KRB5_CCACHE_NAME_X)) {
	char *str;

	maj_stat = get_string(minor_status, value, &str);
	if (maj_stat != GSS_S_COMPLETE)
	    return maj_stat;
	if (str == NULL) {
	    *minor_status = 0;
	    return GSS_S_CALL_INACCESSIBLE_READ;
	}

	*minor_status = krb5_cc_set_default_name(context, str);
	free(str);
	if (*minor_status)
	    return GSS_S_FAILURE;

	return GSS_S_COMPLETE;
    } else if (gss_oid_equal(desired_object, GSS_KRB5_SET_TIME_OFFSET_X)) {
	OM_uint32 offset;
	time_t t;

	maj_stat = get_int32(minor_status, value, &offset);
	if (maj_stat != GSS_S_COMPLETE)
	    return maj_stat;

	t = time(NULL) + offset;

	krb5_set_real_time(context, t, 0);

	*minor_status = 0;
	return GSS_S_COMPLETE;
    } else if (gss_oid_equal(desired_object, GSS_KRB5_GET_TIME_OFFSET_X)) {
	krb5_timestamp sec;
	int32_t usec;
	time_t t;

	t = time(NULL);

	krb5_us_timeofday (context, &sec, &usec);

	maj_stat = set_int32(minor_status, value, sec - t);
	if (maj_stat != GSS_S_COMPLETE)
	    return maj_stat;

	*minor_status = 0;
	return GSS_S_COMPLETE;
    } else if (gss_oid_equal(desired_object, GSS_KRB5_PLUGIN_REGISTER_X)) {
	struct gsskrb5_krb5_plugin c;

	if (value->length != sizeof(c)) {
	    *minor_status = EINVAL;
	    return GSS_S_FAILURE;
	}
	memcpy(&c, value->value, sizeof(c));
	krb5_plugin_register(context, c.type, c.name, c.symbol);

	*minor_status = 0;
	return GSS_S_COMPLETE;
    }

    *minor_status = EINVAL;
    return GSS_S_FAILURE;
}