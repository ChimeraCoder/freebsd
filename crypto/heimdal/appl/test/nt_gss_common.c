
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

#include "test_locl.h"
#include <gssapi.h>
#include "nt_gss_common.h"

RCSID("$Id$");

/*
 * These are functions that are needed to interoperate with the
 * `Sample SSPI Code' in Windows 2000 RC1 SDK.
 */

/*
 * Write the `gss_buffer_t' in `buf' onto the fd `sock', but remember that
 * the length is written in little-endian-order.
 */

void
nt_write_token (int sock, gss_buffer_t buf)
{
    unsigned char net_len[4];
    uint32_t len;
    OM_uint32 min_stat;

    len = buf->length;

    net_len[0] = (len >>  0) & 0xFF;
    net_len[1] = (len >>  8) & 0xFF;
    net_len[2] = (len >> 16) & 0xFF;
    net_len[3] = (len >> 24) & 0xFF;

    if (write (sock, net_len, 4) != 4)
	err (1, "write");
    if (write (sock, buf->value, len) != len)
	err (1, "write");

    gss_release_buffer (&min_stat, buf);
}

/*
 *
 */

void
nt_read_token (int sock, gss_buffer_t buf)
{
    unsigned char net_len[4];
    uint32_t len;

    if (read(sock, net_len, 4) != 4)
	err (1, "read");
    len = (net_len[0] <<  0)
	| (net_len[1] <<  8)
	| (net_len[2] << 16)
	| (net_len[3] << 24);

    buf->length = len;
    buf->value  = malloc(len);
    if (read (sock, buf->value, len) != len)
	err (1, "read");
}

void
gss_print_errors (int min_stat)
{
    OM_uint32 new_stat;
    OM_uint32 msg_ctx = 0;
    gss_buffer_desc status_string;
    OM_uint32 ret;

    do {
	ret = gss_display_status (&new_stat,
				  min_stat,
				  GSS_C_MECH_CODE,
				  GSS_C_NO_OID,
				  &msg_ctx,
				  &status_string);
	fprintf (stderr, "%.*s\n",
		(int)status_string.length,
		(char *)status_string.value);
	gss_release_buffer (&new_stat, &status_string);
    } while (!GSS_ERROR(ret) && msg_ctx != 0);
}

void
gss_verr(int exitval, int status, const char *fmt, va_list ap)
{
    vwarnx (fmt, ap);
    gss_print_errors (status);
    exit (exitval);
}

void
gss_err(int exitval, int status, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    gss_verr (exitval, status, fmt, args);
    va_end(args);
}