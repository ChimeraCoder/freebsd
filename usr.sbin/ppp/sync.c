
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
#include "defs.h"
#include "mbuf.h"
#include "log.h"
#include "sync.h"
#include "timer.h"
#include "lqr.h"
#include "hdlc.h"
#include "throughput.h"
#include "fsm.h"
#include "lcp.h"
#include "ccp.h"
#include "link.h"
#include "async.h"
#include "descriptor.h"
#include "physical.h"

static struct mbuf *
sync_LayerPush(struct bundle *bundle __unused, struct link *l __unused,
	       struct mbuf *bp, int pri __unused, u_short *proto __unused)
{
  log_DumpBp(LogSYNC, "Write", bp);
  m_settype(bp, MB_SYNCOUT);
  bp->priv = 0;
  return bp;
}

static struct mbuf *
sync_LayerPull(struct bundle *b __unused, struct link *l, struct mbuf *bp,
               u_short *proto __unused)
{
  struct physical *p = link2physical(l);
  int len;

  if (!p)
    log_Printf(LogERROR, "Can't Pull a sync packet from a logical link\n");
  else {
    log_DumpBp(LogSYNC, "Read", bp);

    /* Either done here or by the HDLC layer */
    len = m_length(bp);
    p->hdlc.lqm.ifInOctets += len + 1;		/* plus 1 flag octet! */
    p->hdlc.lqm.lqr.InGoodOctets += len + 1;	/* plus 1 flag octet! */
    p->hdlc.lqm.ifInUniPackets++;
    m_settype(bp, MB_SYNCIN);
  }

  return bp;
}

struct layer synclayer = { LAYER_SYNC, "sync", sync_LayerPush, sync_LayerPull };