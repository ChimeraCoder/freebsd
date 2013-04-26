
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

#include "krb5/gsskrb5_locl.h"
#include <err.h>
#include "test_common.h"

char *
gssapi_err(OM_uint32 maj_stat, OM_uint32 min_stat, gss_OID mech)
{
	OM_uint32 disp_min_stat, disp_maj_stat;
	gss_buffer_desc maj_error_message;
	gss_buffer_desc min_error_message;
	OM_uint32 msg_ctx = 0;

	char *ret = NULL;

	maj_error_message.length = 0;
	maj_error_message.value = NULL;
	min_error_message.length = 0;
	min_error_message.value = NULL;

	disp_maj_stat = gss_display_status(&disp_min_stat, maj_stat,
					   GSS_C_GSS_CODE,
					   mech, &msg_ctx, &maj_error_message);
	disp_maj_stat = gss_display_status(&disp_min_stat, min_stat,
					   GSS_C_MECH_CODE,
					   mech, &msg_ctx, &min_error_message);
	if (asprintf(&ret, "gss-code: %lu %.*s -- mech-code: %lu %.*s",
		     (unsigned long)maj_stat,
		     (int)maj_error_message.length,
		     (char *)maj_error_message.value,
		     (unsigned long)min_stat,
		     (int)min_error_message.length,
		     (char *)min_error_message.value) < 0 || ret == NULL)
	    errx(1, "malloc");

	gss_release_buffer(&disp_min_stat, &maj_error_message);
	gss_release_buffer(&disp_min_stat, &min_error_message);

	return ret;
}