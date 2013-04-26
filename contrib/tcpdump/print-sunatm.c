
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
	North Dakota State University
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef lint
static const char rcsid[] _U_ =
    "@(#) $Header: /tcpdump/master/tcpdump/print-sunatm.c,v 1.8 2004-03-17 23:24:38 guy Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>
 
struct mbuf;
struct rtentry;
 
#include <stdio.h>
#include <pcap.h>

#include "interface.h"
#include "extract.h"
#include "addrtoname.h"

#include "atm.h"
#include "atmuni31.h"

/* SunATM header for ATM packet */
#define DIR_POS		0	/* Direction (0x80 = transmit, 0x00 = receive) */
#define VPI_POS		1	/* VPI */
#define VCI_POS		2	/* VCI */
#define PKT_BEGIN_POS   4	/* Start of the ATM packet */

/* Protocol type values in the bottom for bits of the byte at SUNATM_DIR_POS. */
#define PT_LANE		0x01	/* LANE */
#define PT_LLC		0x02	/* LLC encapsulation */

/*
 * This is the top level routine of the printer.  'p' points
 * to the SunATM pseudo-header for the packet, 'h->ts' is the timestamp,
 * 'h->len' is the length of the packet off the wire, and 'h->caplen'
 * is the number of bytes actually captured.
 */
u_int
sunatm_if_print(const struct pcap_pkthdr *h, const u_char *p)
{
	u_int caplen = h->caplen;
	u_int length = h->len;
	u_short vci;
	u_char vpi;
	u_int traftype;

	if (caplen < PKT_BEGIN_POS) {
		printf("[|atm]");
		return (caplen);
	}

	if (eflag) {
		if (p[DIR_POS] & 0x80)
			printf("Tx: ");
		else
			printf("Rx: ");
	}

	switch (p[DIR_POS] & 0x0f) {

	case PT_LANE:
		traftype = ATM_LANE;
		break;

	case PT_LLC:
		traftype = ATM_LLC;
		break;

	default:
		traftype = ATM_UNKNOWN;
		break;
	}

	vci = EXTRACT_16BITS(&p[VCI_POS]);
	vpi = p[VPI_POS];

	p += PKT_BEGIN_POS;
	caplen -= PKT_BEGIN_POS;
	length -= PKT_BEGIN_POS;
	atm_print(vpi, vci, traftype, p, length, caplen);

	return (PKT_BEGIN_POS);
}