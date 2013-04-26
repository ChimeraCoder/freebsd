
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
gss_get_mic(OM_uint32 *minor_status,
    const gss_ctx_id_t ctx,
    gss_qop_t qop_req,
    const gss_buffer_t message_buffer,
    gss_buffer_t message_token)
{
	OM_uint32 maj_stat;
	struct mbuf *m, *mic;

	if (!ctx) {
		*minor_status = 0;
		return (GSS_S_NO_CONTEXT);
	}

	MGET(m, M_WAITOK, MT_DATA);
	if (message_buffer->length > MLEN)
		MCLGET(m, M_WAITOK);
	m_append(m, message_buffer->length, message_buffer->value);

	maj_stat = KGSS_GET_MIC(ctx, minor_status, qop_req, m, &mic);

	m_freem(m);
	if (maj_stat == GSS_S_COMPLETE) {
		message_token->length = m_length(mic, NULL);
		message_token->value = malloc(message_token->length,
		    M_GSSAPI, M_WAITOK);
		m_copydata(mic, 0, message_token->length,
		    message_token->value);
		m_freem(mic);
	}

	return (maj_stat);
}

OM_uint32
gss_get_mic_mbuf(OM_uint32 *minor_status, const gss_ctx_id_t ctx,
    gss_qop_t qop_req, struct mbuf *m, struct mbuf **micp)
{

	if (!ctx) {
		*minor_status = 0;
		return (GSS_S_NO_CONTEXT);
	}

	return (KGSS_GET_MIC(ctx, minor_status, qop_req, m, micp));
}