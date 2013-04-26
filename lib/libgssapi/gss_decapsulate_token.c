
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
#include <stdlib.h>
#include <string.h>

#include "utils.h"

OM_uint32
gss_decapsulate_token(const gss_buffer_t input_token, gss_OID oid,
    gss_buffer_t output_token)
{
	unsigned char *p = input_token->value;
	size_t len = input_token->length;
	size_t a, b;
	gss_OID_desc mech_oid;

	_gss_buffer_zero(output_token);

	/*
	 * Token must start with [APPLICATION 0] SEQUENCE.
	 */
	if (len == 0 || *p != 0x60)
		return (GSS_S_DEFECTIVE_TOKEN);
	p++;
	len--;

	/*
	 * Decode the length and make sure it agrees with the
	 * token length.
	 */
	if (len == 0)
		return (GSS_S_DEFECTIVE_TOKEN);
	if ((*p & 0x80) == 0) {
		a = *p;
		p++;
		len--;
	} else {
		b = *p & 0x7f;
		p++;
		len--;
		if (len < b)
			return (GSS_S_DEFECTIVE_TOKEN);
		a = 0;
		while (b) {
			a = (a << 8) | *p;
			p++;
			len--;
			b--;
		}
	}
	if (a != len)
		return (GSS_S_DEFECTIVE_TOKEN);

	/*
	 * Decode the OID for the mechanism. Simplify life by
	 * assuming that the OID length is less than 128 bytes.
	 */
	if (len < 2 || *p != 0x06)
		return (GSS_S_DEFECTIVE_TOKEN);
	if ((p[1] & 0x80) || p[1] > (len - 2))
		return (GSS_S_DEFECTIVE_TOKEN);
	mech_oid.length = p[1];
	p += 2;
	len -= 2;
	mech_oid.elements = p;

	if (!gss_oid_equal(&mech_oid, oid))
		return (GSS_S_FAILURE);

	p += mech_oid.length;
	len -= mech_oid.length;

	output_token->length = len;
	output_token->value = malloc(len);
	if (!output_token->value)
		return (GSS_S_DEFECTIVE_TOKEN);
	memcpy(output_token->value, p, len);

	return (GSS_S_COMPLETE);
}