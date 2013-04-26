
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

#include "includes.h"

#include <sys/types.h>

#include <signal.h>
#include <stdarg.h>

#include "ssh1.h"
#include "ssh2.h"
#include "log.h"
#include "dispatch.h"
#include "packet.h"
#include "compat.h"

#define DISPATCH_MAX	255

dispatch_fn *dispatch[DISPATCH_MAX];

void
dispatch_protocol_error(int type, u_int32_t seq, void *ctxt)
{
	logit("dispatch_protocol_error: type %d seq %u", type, seq);
	if (!compat20)
		fatal("protocol error");
	packet_start(SSH2_MSG_UNIMPLEMENTED);
	packet_put_int(seq);
	packet_send();
	packet_write_wait();
}
void
dispatch_protocol_ignore(int type, u_int32_t seq, void *ctxt)
{
	logit("dispatch_protocol_ignore: type %d seq %u", type, seq);
}
void
dispatch_init(dispatch_fn *dflt)
{
	u_int i;
	for (i = 0; i < DISPATCH_MAX; i++)
		dispatch[i] = dflt;
}
void
dispatch_range(u_int from, u_int to, dispatch_fn *fn)
{
	u_int i;

	for (i = from; i <= to; i++) {
		if (i >= DISPATCH_MAX)
			break;
		dispatch[i] = fn;
	}
}
void
dispatch_set(int type, dispatch_fn *fn)
{
	dispatch[type] = fn;
}
void
dispatch_run(int mode, volatile sig_atomic_t *done, void *ctxt)
{
	for (;;) {
		int type;
		u_int32_t seqnr;

		if (mode == DISPATCH_BLOCK) {
			type = packet_read_seqnr(&seqnr);
		} else {
			type = packet_read_poll_seqnr(&seqnr);
			if (type == SSH_MSG_NONE)
				return;
		}
		if (type > 0 && type < DISPATCH_MAX && dispatch[type] != NULL)
			(*dispatch[type])(type, seqnr, ctxt);
		else
			packet_disconnect("protocol error: rcvd type %d", type);
		if (done != NULL && *done)
			return;
	}
}