
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

#include <gssapi/gssapi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "mech_switch.h"
#include "utils.h"

static const char *
calling_error(OM_uint32 v)
{
    static const char *msgs[] = {
	NULL,			/* 0 */
	"A required input parameter could not be read.", /*  */
	"A required output parameter could not be written.", /*  */
	"A parameter was malformed"
    };

    v >>= GSS_C_CALLING_ERROR_OFFSET;

    if (v == 0)
	return "";
    else if (v >= sizeof(msgs)/sizeof(*msgs))
	return "unknown calling error";
    else
	return msgs[v];
}

static const char *
routine_error(OM_uint32 v)
{
    static const char *msgs[] = {
	"Function completed successfully",			/* 0 */
	"An unsupported mechanism was requested",
	"An invalid name was supplied",
	"A supplied name was of an unsupported type",
	"Incorrect channel bindings were supplied",
	"An invalid status code was supplied",
	"A token had an invalid MIC",
	"No credentials were supplied, "
	"or the credentials were unavailable or inaccessible.",
	"No context has been established",
	"A token was invalid",
	"A credential was invalid",
	"The referenced credentials have expired",
	"The context has expired",
	"Miscellaneous failure (see text)",
	"The quality-of-protection requested could not be provide",
	"The operation is forbidden by local security policy",
	"The operation or option is not available",
	"The requested credential element already exists",
	"The provided name was not a mechanism name.",
    };

    v >>= GSS_C_ROUTINE_ERROR_OFFSET;

    if (v >= sizeof(msgs)/sizeof(*msgs))
	return "unknown routine error";
    else
	return msgs[v];
}

static const char *
supplementary_error(OM_uint32 v)
{
    static const char *msgs[] = {
	"normal completion",
	"continuation call to routine required",
	"duplicate per-message token detected",
	"timed-out per-message token detected",
	"reordered (early) per-message token detected",
	"skipped predecessor token(s) detected"
    };

    v >>= GSS_C_SUPPLEMENTARY_OFFSET;

    if (v >= sizeof(msgs)/sizeof(*msgs))
	return "unknown routine error";
    else
	return msgs[v];
}

#if defined(__NO_TLS)

/*
 * These platforms don't support TLS on FreeBSD - threads will just
 * have to step on each other's error values for now.
 */
#define __thread

#endif

struct mg_thread_ctx {
    gss_OID mech;
    OM_uint32 maj_stat;
    OM_uint32 min_stat;
    gss_buffer_desc maj_error;
    gss_buffer_desc min_error;
};
static __thread struct mg_thread_ctx last_error_context;

static OM_uint32
_gss_mg_get_error(const gss_OID mech, OM_uint32 type,
		  OM_uint32 value, gss_buffer_t string)
{
	struct mg_thread_ctx *mg;

	mg = &last_error_context;

	if (mech != NULL && gss_oid_equal(mg->mech, mech) == 0)
		return (GSS_S_BAD_STATUS);

	switch (type) {
	case GSS_C_GSS_CODE: {
		if (value != mg->maj_stat || mg->maj_error.length == 0)
			break;
		string->value = malloc(mg->maj_error.length);
		string->length = mg->maj_error.length;
		memcpy(string->value, mg->maj_error.value,
		    mg->maj_error.length);
		return (GSS_S_COMPLETE);
	}
	case GSS_C_MECH_CODE: {
		if (value != mg->min_stat || mg->min_error.length == 0)
			break;
		string->value = malloc(mg->min_error.length);
		string->length = mg->min_error.length;
		memcpy(string->value, mg->min_error.value,
		    mg->min_error.length);
		return (GSS_S_COMPLETE);
	}
	}
	string->value = NULL;
	string->length = 0;
	return (GSS_S_BAD_STATUS);
}

void
_gss_mg_error(struct _gss_mech_switch *m, OM_uint32 maj, OM_uint32 min)
{
	OM_uint32 major_status, minor_status;
	OM_uint32 message_content;
	struct mg_thread_ctx *mg;

	mg = &last_error_context;

	gss_release_buffer(&minor_status, &mg->maj_error);
	gss_release_buffer(&minor_status, &mg->min_error);

	mg->mech = &m->gm_mech_oid;
	mg->maj_stat = maj;
	mg->min_stat = min;

	major_status = m->gm_display_status(&minor_status,
	    maj, 
	    GSS_C_GSS_CODE,
	    &m->gm_mech_oid,
	    &message_content,
	    &mg->maj_error);
	if (GSS_ERROR(major_status)) {
		mg->maj_error.value = NULL;
		mg->maj_error.length = 0;
	}
	major_status = m->gm_display_status(&minor_status,
	    min, 
	    GSS_C_MECH_CODE,
	    &m->gm_mech_oid,
	    &message_content,
	    &mg->min_error);
	if (GSS_ERROR(major_status)) {
		mg->min_error.value = NULL;
		mg->min_error.length = 0;
	}
}

OM_uint32
gss_display_status(OM_uint32 *minor_status,
    OM_uint32 status_value,
    int status_type,
    const gss_OID mech_type,
    OM_uint32 *message_content,
    gss_buffer_t status_string)
{
	OM_uint32 major_status;

	_gss_buffer_zero(status_string);
	*message_content = 0;

	major_status = _gss_mg_get_error(mech_type, status_type,
					 status_value, status_string);
	if (major_status == GSS_S_COMPLETE) {

		*message_content = 0;
		*minor_status = 0;
		return (GSS_S_COMPLETE);
	}

	*minor_status = 0;
	switch (status_type) {
	case GSS_C_GSS_CODE: {
		char *buf;

		if (GSS_SUPPLEMENTARY_INFO(status_value))
		    asprintf(&buf, "%s", supplementary_error(
		        GSS_SUPPLEMENTARY_INFO(status_value)));
		else
		    asprintf (&buf, "%s %s",
		        calling_error(GSS_CALLING_ERROR(status_value)),
			routine_error(GSS_ROUTINE_ERROR(status_value)));

		if (buf == NULL)
			break;

		status_string->length = strlen(buf);
		status_string->value  = buf;

		return (GSS_S_COMPLETE);
	}
	case GSS_C_MECH_CODE: {
		OM_uint32 maj_junk, min_junk;
		gss_buffer_desc oid;
		char *buf;

		maj_junk = gss_oid_to_str(&min_junk, mech_type, &oid);
		if (maj_junk != GSS_S_COMPLETE) {
			oid.value = strdup("unknown");
			oid.length = 7;
		}

		asprintf (&buf, "unknown mech-code %lu for mech %.*s",
			  (unsigned long)status_value,
			  (int)oid.length, (char *)oid.value);
		if (maj_junk == GSS_S_COMPLETE)
			gss_release_buffer(&min_junk, &oid);

		if (buf == NULL)
		    break;

		status_string->length = strlen(buf);
		status_string->value  = buf;

		return (GSS_S_COMPLETE);
	}
	}
	_gss_buffer_zero(status_string);
	return (GSS_S_BAD_STATUS);
}

void
_gss_mg_collect_error(gss_OID mech, OM_uint32 maj, OM_uint32 min)
{
	struct _gss_mech_switch *m;

	m = _gss_find_mech_switch(mech);
	if (m != NULL)
		_gss_mg_error(m, maj, min);
}