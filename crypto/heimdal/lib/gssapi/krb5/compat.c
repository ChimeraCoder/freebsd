
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

#include "gsskrb5_locl.h"

static krb5_error_code
check_compat(OM_uint32 *minor_status,
	     krb5_context context, krb5_const_principal name,
	     const char *option, krb5_boolean *compat,
	     krb5_boolean match_val)
{
    krb5_error_code ret = 0;
    char **p, **q;
    krb5_principal match;


    p = krb5_config_get_strings(context, NULL, "gssapi",
				option, NULL);
    if(p == NULL)
	return 0;

    match = NULL;
    for(q = p; *q; q++) {
	ret = krb5_parse_name(context, *q, &match);
	if (ret)
	    break;

	if (krb5_principal_match(context, name, match)) {
	    *compat = match_val;
	    break;
	}

	krb5_free_principal(context, match);
	match = NULL;
    }
    if (match)
	krb5_free_principal(context, match);
    krb5_config_free_strings(p);

    if (ret) {
	if (minor_status)
	    *minor_status = ret;
	return GSS_S_FAILURE;
    }

    return 0;
}

/*
 * ctx->ctx_id_mutex is assumed to be locked
 */

OM_uint32
_gss_DES3_get_mic_compat(OM_uint32 *minor_status,
			 gsskrb5_ctx ctx,
			 krb5_context context)
{
    krb5_boolean use_compat = FALSE;
    OM_uint32 ret;

    if ((ctx->more_flags & COMPAT_OLD_DES3_SELECTED) == 0) {
	ret = check_compat(minor_status, context, ctx->target,
			   "broken_des3_mic", &use_compat, TRUE);
	if (ret)
	    return ret;
	ret = check_compat(minor_status, context, ctx->target,
			   "correct_des3_mic", &use_compat, FALSE);
	if (ret)
	    return ret;

	if (use_compat)
	    ctx->more_flags |= COMPAT_OLD_DES3;
	ctx->more_flags |= COMPAT_OLD_DES3_SELECTED;
    }
    return 0;
}

#if 0
OM_uint32
gss_krb5_compat_des3_mic(OM_uint32 *minor_status, gss_ctx_id_t ctx, int on)
{
    *minor_status = 0;

    HEIMDAL_MUTEX_lock(&ctx->ctx_id_mutex);
    if (on) {
	ctx->more_flags |= COMPAT_OLD_DES3;
    } else {
	ctx->more_flags &= ~COMPAT_OLD_DES3;
    }
    ctx->more_flags |= COMPAT_OLD_DES3_SELECTED;
    HEIMDAL_MUTEX_unlock(&ctx->ctx_id_mutex);

    return 0;
}
#endif