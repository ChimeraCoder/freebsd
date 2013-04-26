
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/kobj.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>

#include <kgssapi/gssapi.h>
#include <kgssapi/gssapi_impl.h>

#include "kgss_if.h"

OM_uint32
gss_unwrap(OM_uint32 *minor_status,
    const gss_ctx_id_t ctx,
    const gss_buffer_t input_message_buffer,
    gss_buffer_t output_message_buffer,
    int *conf_state,
    gss_qop_t *qop_state)
{
	OM_uint32 maj_stat;
	struct mbuf *m;

	if (!ctx) {
		*minor_status = 0;
		return (GSS_S_NO_CONTEXT);
	}

	MGET(m, M_WAITOK, MT_DATA);
	if (input_message_buffer->length > MLEN)
		MCLGET(m, M_WAITOK);
	m_append(m, input_message_buffer->length, input_message_buffer->value);

	maj_stat = KGSS_UNWRAP(ctx, minor_status, &m, conf_state, qop_state);

	/*
	 * On success, m is the wrapped message, on failure, m is
	 * freed.
	 */
	if (maj_stat == GSS_S_COMPLETE) {
		output_message_buffer->length = m_length(m, NULL);
		output_message_buffer->value =
			malloc(output_message_buffer->length,
			    M_GSSAPI, M_WAITOK);
		m_copydata(m, 0, output_message_buffer->length,
		    output_message_buffer->value);
		m_freem(m);
	}

	return (maj_stat);
}

OM_uint32
gss_unwrap_mbuf(OM_uint32 *minor_status,
    const gss_ctx_id_t ctx,
    struct mbuf **mp,
    int *conf_state,
    gss_qop_t *qop_state)
{

	if (!ctx) {
		*minor_status = 0;
		return (GSS_S_NO_CONTEXT);
	}

	return (KGSS_UNWRAP(ctx, minor_status, mp, conf_state, qop_state));
}