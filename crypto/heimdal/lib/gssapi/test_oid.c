
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <roken.h>
#include <stdio.h>
#include <gssapi.h>
#include <gssapi_krb5.h>
#include <gssapi_spnego.h>
#include <err.h>

int
main(int argc, char **argv)
{
    OM_uint32 minor_status, maj_stat;
    gss_buffer_desc data;
    int ret;

    maj_stat = gss_oid_to_str(&minor_status, GSS_KRB5_MECHANISM, &data);
    if (GSS_ERROR(maj_stat))
	errx(1, "gss_oid_to_str failed");

    ret = strncmp(data.value, "1 2 840 113554 1 2 2", data.length);
    gss_release_buffer(&maj_stat, &data);
    if (ret)
	return 1;

    maj_stat = gss_oid_to_str(&minor_status, GSS_C_NT_EXPORT_NAME, &data);
    if (GSS_ERROR(maj_stat))
	errx(1, "gss_oid_to_str failed");

    ret = strncmp(data.value, "1 3 6 1 5 6 4", data.length);
    gss_release_buffer(&maj_stat, &data);
    if (ret)
	return 1;

    return 0;
}