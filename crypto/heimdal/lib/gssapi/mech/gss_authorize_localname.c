
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

#include "mech_locl.h"

gss_buffer_desc GSSAPI_LIB_VARIABLE __gss_c_attr_local_login_user =  {
    sizeof("local-login-user") - 1,
    "local-login-user"
};

static OM_uint32
mech_authorize_localname(OM_uint32 *minor_status,
	                 const struct _gss_name *name,
	                 const struct _gss_name *user)
{
    OM_uint32 major_status = GSS_S_NAME_NOT_MN;
    struct _gss_mechanism_name *mn;

    HEIM_SLIST_FOREACH(mn, &name->gn_mn, gmn_link) {
        gssapi_mech_interface m = mn->gmn_mech;

        if (m->gm_authorize_localname == NULL) {
            major_status = GSS_S_UNAVAILABLE;
            continue;
        }

        major_status = m->gm_authorize_localname(minor_status,
                                                 mn->gmn_name,
                                                 &user->gn_value,
                                                 &user->gn_type);
        if (major_status != GSS_S_UNAUTHORIZED)
            break;
    }

    return major_status;
}

/*
 * Naming extensions based local login authorization.
 */
static OM_uint32
attr_authorize_localname(OM_uint32 *minor_status,
	                 const struct _gss_name *name,
	                 const struct _gss_name *user)
{
    OM_uint32 major_status = GSS_S_UNAVAILABLE;
    int more = -1;

    if (!gss_oid_equal(&user->gn_type, GSS_C_NT_USER_NAME))
        return GSS_S_BAD_NAMETYPE;

    while (more != 0 && major_status != GSS_S_COMPLETE) {
	OM_uint32 tmpMajor, tmpMinor;
	gss_buffer_desc value;
	gss_buffer_desc display_value;
	int authenticated = 0, complete = 0;

	tmpMajor = gss_get_name_attribute(minor_status,
					  (gss_name_t)name,
					  GSS_C_ATTR_LOCAL_LOGIN_USER,
					  &authenticated,
					  &complete,
					  &value,
					  &display_value,
					  &more);
	if (GSS_ERROR(tmpMajor)) {
	    major_status = tmpMajor;
	    break;
	}

	/* If attribute is present, return an authoritative error code. */
	if (authenticated &&
	    value.length == user->gn_value.length &&
	    memcmp(value.value, user->gn_value.value, user->gn_value.length) == 0)
	    major_status = GSS_S_COMPLETE;
	else
	    major_status = GSS_S_UNAUTHORIZED;

	gss_release_buffer(&tmpMinor, &value);
	gss_release_buffer(&tmpMinor, &display_value);
    }

    return major_status;
}

GSSAPI_LIB_FUNCTION OM_uint32 GSSAPI_LIB_CALL
gss_authorize_localname(OM_uint32 *minor_status,
	                const gss_name_t gss_name,
	                const gss_name_t gss_user)

{
    OM_uint32 major_status;
    const struct _gss_name *name = (const struct _gss_name *) gss_name;
    const struct _gss_name *user = (const struct _gss_name *) gss_user;
    int mechAvailable = 0;

    *minor_status = 0;

    if (gss_name == GSS_C_NO_NAME || gss_user == GSS_C_NO_NAME)
        return GSS_S_CALL_INACCESSIBLE_READ;

    /*
     * We should check that the user name is not a mechanism name, but
     * as Heimdal always calls the mechanism's gss_import_name(), it's
     * not possible to make this check.
     */
#if 0
    if (HEIM_SLIST_FIRST(&user->gn_mn) != NULL)
        return GSS_S_BAD_NAME;
#endif

    /* If mech returns yes, we return yes */
    major_status = mech_authorize_localname(minor_status, name, user);
    if (major_status == GSS_S_COMPLETE)
	return GSS_S_COMPLETE;
    else if (major_status != GSS_S_UNAVAILABLE)
	mechAvailable = 1;

    /* If attribute exists, it is authoritative */
    major_status = attr_authorize_localname(minor_status, name, user);
    if (major_status == GSS_S_COMPLETE || major_status == GSS_S_UNAUTHORIZED)
	return major_status;

    /* If mechanism did not implement SPI, compare the local name */
    if (mechAvailable == 0) {
	int match = 0;

        major_status = gss_compare_name(minor_status, gss_name,
                                        gss_user, &match);
	if (major_status == GSS_S_COMPLETE && match == 0)
	    major_status = GSS_S_UNAUTHORIZED;
    }

    return major_status;
}

GSSAPI_LIB_FUNCTION int GSSAPI_LIB_CALL
gss_userok(const gss_name_t name,
           const char *user)
{
    OM_uint32 major_status, minor_status;
    gss_buffer_desc userBuf;
    gss_name_t userName;

    userBuf.value = (void *)user;
    userBuf.length = strlen(user);

    major_status = gss_import_name(&minor_status, &userBuf,
                                   GSS_C_NT_USER_NAME, &userName);
    if (GSS_ERROR(major_status))
        return 0;

    major_status = gss_authorize_localname(&minor_status, name, userName);

    gss_release_name(&minor_status, &userName);

    return (major_status == GSS_S_COMPLETE);
}