
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
gss_encapsulate_token(const gss_buffer_t input_token, gss_OID oid,
    gss_buffer_t output_token)
{
	unsigned char *p;
	size_t len, inside_len;
	size_t a, b;
	int i;

	_gss_buffer_zero(output_token);

	/*
	 * First time around, we calculate the size, second time, we
	 * encode the token.
	 */
	p = 0;
	for (i = 0; i < 2; i++) {
		len = 0;

		/*
		 * Token starts with [APPLICATION 0] SEQUENCE.
		 */
		if (p)
			*p++ = 0x60;
		len++;

		/*
		 * The length embedded in the token is the space
		 * needed for the encapsulated oid plus the length of
		 * the inner token.
		 */
		if (oid->length > 127)
			return (GSS_S_DEFECTIVE_TOKEN);

		inside_len = 2 + oid->length + input_token->length;

		/*
		 * Figure out how to encode the length
		 */
		if (inside_len < 128) {
			if (p)
				*p++ = inside_len;
			len++;
		} else {
			b = 1;
			if (inside_len >= 0x100)
				b++;
			if (inside_len >= 0x10000)
				b++;
			if (inside_len >= 0x1000000)
				b++;
			if (p)
				*p++ = b | 0x80;
			len++;
			a = inside_len << 8*(4 - b);
			while (b) {
				if (p)
					*p++ = (a >> 24);
				a <<= 8;
				len++;
				b--;
			}
		}

		/*
		 * Encode the OID for the mechanism. Simplify life by
		 * assuming that the OID length is less than 128 bytes.
		 */
		if (p)
			*p++ = 0x06;
		len++;
		if (p)
			*p++ = oid->length;
		len++;
		if (p) {
			memcpy(p, oid->elements, oid->length);
			p += oid->length;
		}
		len += oid->length;

		if (p) {
			memcpy(p, input_token->value, input_token->length);
			p += input_token->length;
		}
		len += input_token->length;

		if (i == 0) {
			output_token->length = len;
			output_token->value = malloc(len);
			if (!output_token->value)
				return (GSS_S_DEFECTIVE_TOKEN);
			p = output_token->value;
		}
	}

	return (GSS_S_COMPLETE);
}