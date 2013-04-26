
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

#include "defs.h"
#include "layer.h"
#include "timer.h"
#include "fsm.h"
#include "log.h"
#include "mbuf.h"
#include "acf.h"
#include "proto.h"
#include "throughput.h"
#include "lqr.h"
#include "hdlc.h"
#include "lcp.h"
#include "ccp.h"
#include "link.h"
#include "descriptor.h"
#include "async.h"
#include "physical.h"

int
acf_WrapperOctets(struct lcp *lcp, u_short proto)
{
  return (proto == PROTO_LCP || lcp->his_acfcomp == 0) ? 2 : 0;
}

static struct mbuf *
acf_LayerPush(struct bundle *b __unused, struct link *l, struct mbuf *bp,
              int pri __unused, u_short *proto)
{
  const u_char cp[2] = { HDLC_ADDR, HDLC_UI };

  if (*proto == PROTO_LCP || l->lcp.his_acfcomp == 0) {
    bp = m_prepend(bp, cp, 2, 0);
    m_settype(bp, MB_ACFOUT);
  }

  return bp;
}

static struct mbuf *
acf_LayerPull(struct bundle *b __unused, struct link *l, struct mbuf *bp,
	      u_short *proto __unused)
{
  struct physical *p = link2physical(l);
  u_char cp[2];

  if (!p) {
    log_Printf(LogERROR, "Can't Pull an acf packet from a logical link\n");
    return bp;
  }

  if (mbuf_View(bp, cp, 2) == 2) {
    if (!p->link.lcp.want_acfcomp) {
      /* We expect the packet not to be compressed */
      bp = mbuf_Read(bp, cp, 2);
      if (cp[0] != HDLC_ADDR) {
        p->hdlc.lqm.ifInErrors++;
        p->hdlc.stats.badaddr++;
        log_Printf(LogDEBUG, "acf_LayerPull: addr 0x%02x\n", cp[0]);
        m_freem(bp);
        return NULL;
      }
      if (cp[1] != HDLC_UI) {
        p->hdlc.lqm.ifInErrors++;
        p->hdlc.stats.badcommand++;
        log_Printf(LogDEBUG, "acf_LayerPull: control 0x%02x\n", cp[1]);
        m_freem(bp);
        return NULL;
      }
      m_settype(bp, MB_ACFIN);
    } else if (cp[0] == HDLC_ADDR && cp[1] == HDLC_UI) {
      /*
       * We can receive compressed packets, but the peer still sends
       * uncompressed packets (or maybe this is a PROTO_LCP packet) !
       */
      bp = mbuf_Read(bp, cp, 2);
      m_settype(bp, MB_ACFIN);
    }
  }

  return bp;
}

struct layer acflayer = { LAYER_ACF, "acf", acf_LayerPush, acf_LayerPull };