
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
     * We cant check the mech here since a pseudo-mech might have
     * called an lower layer and then the mech info is all broken
     */    if (mech != NULL && gss_oid_equal(mg->mech, mech) == 0)
	return GSS_S_BAD_STATUS;
#endif

    switch (type) {
    case GSS_C_GSS_CODE: {
	if (value != mg->maj_stat || mg->maj_error.length == 0)
	    break;
	string->value = malloc(mg->maj_error.length + 1);
	string->length = mg->maj_error.length;
	memcpy(string->value, mg->maj_error.value, mg->maj_error.length);
        ((char *) string->value)[string->length] = '\0';
	return GSS_S_COMPLETE;
    }
    case GSS_C_MECH_CODE: {
	if (value != mg->min_stat || mg->min_error.length == 0)
	    break;
	string->value = malloc(mg->min_error.length + 1);
	string->length = mg->min_error.length;
	memcpy(string->value, mg->min_error.value, mg->min_error.length);
        ((char *) string->value)[string->length] = '\0';
	return GSS_S_COMPLETE;
    }
    }
    string->value = NULL;
    string->length = 0;
    return GSS_S_BAD_STATUS;
}

void
_gss_mg_error(gssapi_mech_interface m, OM_uint32 maj, OM_uint32 min)
{
    OM_uint32 major_status, minor_status;
    OM_uint32 message_content;
    struct mg_thread_ctx *mg;

    /*
     * Mechs without gss_display_status() does
     * gss_mg_collect_error() by themself.
     */
    if (m->gm_display_status == NULL)
	return ;

    mg = _gss_mechglue_thread();
    if (mg == NULL)
	return;

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

void
gss_mg_collect_error(gss_OID mech, OM_uint32 maj, OM_uint32 min)
{
    gssapi_mech_interface m = __gss_get_mechanism(mech);
    if (m == NULL)
	return;
    _gss_mg_error(m, maj, min);
}