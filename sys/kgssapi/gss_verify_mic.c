
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
gss_verify_mic(OM_uint32 *minor_status,
    const gss_ctx_id_t ctx,
    const gss_buffer_t message_buffer,
    const gss_buffer_t token_buffer,
    gss_qop_t *qop_state)
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

	MGET(mic, M_WAITOK, MT_DATA);
	if (token_buffer->length > MLEN)
		MCLGET(mic, M_WAITOK);
	m_append(mic, token_buffer->length, token_buffer->value);

	maj_stat = KGSS_VERIFY_MIC(ctx, minor_status, m, mic, qop_state);

	m_freem(m);
	m_freem(mic);

	return (maj_stat);
}

OM_uint32
gss_verify_mic_mbuf(OM_uint32 *minor_status, const gss_ctx_id_t ctx,
    struct mbuf *m, struct mbuf *mic, gss_qop_t *qop_state)
{

	if (!ctx) {
		*minor_status = 0;
		return (GSS_S_NO_CONTEXT);
	}

	return (KGSS_VERIFY_MIC(ctx, minor_status, m, mic, qop_state));
}