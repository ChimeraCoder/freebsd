
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

#include <sys/param.h>

#include <sys/socket.h>
#include <net/route.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/un.h>

#include <termios.h>

#include "layer.h"
#include "defs.h"
#include "log.h"
#include "timer.h"
#include "fsm.h"
#include "mbuf.h"
#include "throughput.h"
#include "lqr.h"
#include "hdlc.h"
#include "lcp.h"
#include "ccp.h"
#include "link.h"
#include "iplist.h"
#include "slcompress.h"
#include "ncpaddr.h"
#include "ipcp.h"
#include "filter.h"
#include "descriptor.h"
#include "mp.h"
#include "iface.h"
#ifndef NORADIUS
#include "radius.h"
#endif
#include "ipv6cp.h"
#include "ncp.h"
#include "bundle.h"


/*-
 * We are in a liberal position about MSS
 * (RFC 879, section 7).
 */
#define MAXMSS(mtu) ((mtu) - sizeof(struct ip) - sizeof(struct tcphdr) - 12)


/*-
 * The following macro is used to update an
 * internet checksum.  "acc" is a 32-bit
 * accumulation of all the changes to the
 * checksum (adding in old 16-bit words and
 * subtracting out new words), and "cksum"
 * is the checksum value to be updated.
 */
#define ADJUST_CHECKSUM(acc, cksum) { \
  acc += cksum; \
  if (acc < 0) { \
    acc = -acc; \
    acc = (acc >> 16) + (acc & 0xffff); \
    acc += acc >> 16; \
    cksum = (u_short) ~acc; \
  } else { \
    acc = (acc >> 16) + (acc & 0xffff); \
    acc += acc >> 16; \
    cksum = (u_short) acc; \
  } \
}

static void
MSSFixup(struct tcphdr *tc, size_t pktlen, u_int16_t maxmss)
{
  size_t hlen, olen, optlen;
  u_char *opt;
  u_int16_t *mss;
  int accumulate;

  hlen = tc->th_off << 2;

  /* Invalid header length or header without options. */
  if (hlen <= sizeof(struct tcphdr) || hlen > pktlen)
    return;

  /* MSS option only allowed within SYN packets. */
  if (!(tc->th_flags & TH_SYN))
    return;

  for (olen = hlen - sizeof(struct tcphdr), opt = (u_char *)(tc + 1);
       olen > 0; olen -= optlen, opt += optlen) {
    if (*opt == TCPOPT_EOL)
      break;
    else if (*opt == TCPOPT_NOP)
      optlen = 1;
    else {
      optlen = *(opt + 1);
      if (optlen <= 0 || optlen > olen)
        break;
      if (*opt == TCPOPT_MAXSEG) {
        if (optlen != TCPOLEN_MAXSEG)
          continue;
        mss = (u_int16_t *)(opt + 2);
        if (ntohs(*mss) > maxmss) {
          log_Printf(LogDEBUG, "MSS: %u -> %u\n",
               ntohs(*mss), maxmss);
          accumulate = *mss;
          *mss = htons(maxmss);
          accumulate -= *mss;
          ADJUST_CHECKSUM(accumulate, tc->th_sum);
        }
      }
    }
  }
}

static struct mbuf *
tcpmss_Check(struct bundle *bundle, struct mbuf *bp)
{
  struct ip *pip;
  size_t hlen, plen;

  if (!Enabled(bundle, OPT_TCPMSSFIXUP))
    return bp;

  bp = m_pullup(bp);
  plen = m_length(bp);
  pip = (struct ip *)MBUF_CTOP(bp);
  hlen = pip->ip_hl << 2;

  /*
   * Check for MSS option only for TCP packets with zero fragment offsets
   * and correct total and header lengths.
   */
  if (pip->ip_p == IPPROTO_TCP && (ntohs(pip->ip_off) & IP_OFFMASK) == 0 &&
      ntohs(pip->ip_len) == plen && hlen <= plen &&
      plen >= sizeof(struct tcphdr) + hlen)
    MSSFixup((struct tcphdr *)(MBUF_CTOP(bp) + hlen), plen - hlen,
             MAXMSS(bundle->iface->mtu));

  return bp;
}

static struct mbuf *
tcpmss_LayerPush(struct bundle *bundle, struct link *l __unused,
		 struct mbuf *bp, int pri __unused, u_short *proto __unused)
{
	return tcpmss_Check(bundle, bp);
}

static struct mbuf *
tcpmss_LayerPull(struct bundle *bundle, struct link *l __unused,
		 struct mbuf *bp, u_short *proto __unused)
{
	return tcpmss_Check(bundle, bp);
}

struct layer tcpmsslayer =
  { LAYER_PROTO, "tcpmss", tcpmss_LayerPush, tcpmss_LayerPull };