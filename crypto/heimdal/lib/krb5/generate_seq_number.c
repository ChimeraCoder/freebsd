
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

#include "krb5_locl.h"

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_generate_seq_number(krb5_context context,
			 const krb5_keyblock *key,
			 uint32_t *seqno)
{
    if (RAND_bytes((void *)seqno, sizeof(*seqno)) <= 0)
	krb5_abortx(context, "Failed to generate random block");
    /* MIT used signed numbers, lets not stomp into that space directly */
    *seqno &= 0x3fffffff;
    if (*seqno == 0)
	*seqno = 1;
    return 0;
}