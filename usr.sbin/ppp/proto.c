
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

#include <sys/types.h>

#include <stdio.h>
#include <termios.h>

#include "layer.h"
#include "acf.h"
#include "defs.h"
#include "log.h"
#include "timer.h"
#include "fsm.h"
#include "mbuf.h"
#include "proto.h"
#include "throughput.h"
#include "lqr.h"
#include "hdlc.h"
#include "lcp.h"
#include "ccp.h"
#include "link.h"

int
proto_WrapperOctets(struct lcp *lcp, u_short proto)
{
  return (lcp->his_protocomp && !(proto & 0xff00)) ? 1 : 2;
}

struct mbuf *
proto_Prepend(struct mbuf *bp, u_short proto, unsigned comp, int extra)
{
  u_char cp[2];

  cp[0] = proto >> 8;
  cp[1] = proto & 0xff;

  if (comp && cp[0] == 0)
    bp = m_prepend(bp, cp + 1, 1, extra);
  else
    bp = m_prepend(bp, cp, 2, extra);

  return bp;
}

static struct mbuf *
proto_LayerPush(struct bundle *b __unused, struct link *l, struct mbuf *bp,
                int pri __unused, u_short *proto)
{
  log_Printf(LogDEBUG, "proto_LayerPush: Using 0x%04x\n", *proto);
  bp = proto_Prepend(bp, *proto, l->lcp.his_protocomp,
                     acf_WrapperOctets(&l->lcp, *proto));
  m_settype(bp, MB_PROTOOUT);
  link_ProtocolRecord(l, *proto, PROTO_OUT);

  return bp;
}

static struct mbuf *
proto_LayerPull(struct bundle *b __unused, struct link *l, struct mbuf *bp,
                u_short *proto)
{
  u_char cp[2];
  size_t got;

  if ((got = mbuf_View(bp, cp, 2)) == 0) {
    m_freem(bp);
    return NULL;
  }

  *proto = cp[0];
  if (!(*proto & 1)) {
    if (got == 1) {
      m_freem(bp);
      return NULL;
    }
    bp = mbuf_Read(bp, cp, 2);
    *proto = (*proto << 8) | cp[1];
  } else
    bp = mbuf_Read(bp, cp, 1);

  log_Printf(LogDEBUG, "proto_LayerPull: unknown -> 0x%04x\n", *proto);
  m_settype(bp, MB_PROTOIN);
  link_ProtocolRecord(l, *proto, PROTO_IN);

  return bp;
}

struct layer protolayer =
  { LAYER_PROTO, "proto", proto_LayerPush, proto_LayerPull };