
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

/*
 * npe statistics class.
 */
#include <sys/types.h>
#include <sys/sysctl.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <err.h>

#include "npestats.h"

#define	AFTER(prev)	((prev)+1)

static const struct fmt npestats[] = {
#define	S_ALIGN		0
	{ 7,	"align",	"align",	"alignment errors" },
#define	S_FCS		AFTER(S_ALIGN)
	{ 7,	"fcs",		"fcs",		"FCS errors" },
#define	S_MACRX		AFTER(S_FCS)
	{ 7,	"macrx",	"macrx",	"internal MAC rx errors" },
#define	S_RXORN		AFTER(S_MACRX)
	{ 6,	"overrun",	"overrun",	"rx overrun discards" },
#define	S_LEARN		AFTER(S_RXORN)
	{ 5,	"learn",	"learn",	"rx learned entry discards" },
#define	S_LARGE		AFTER(S_LEARN)
	{ 5,	"large",	"large",	"rx large frame discards" },
#define	S_STP		AFTER(S_LARGE)
	{ 5,	"stp",		"stp",		"rx STP blocked discards" },
#define	S_RX_VLAN_TYPE	AFTER(S_STP)
	{ 5,	"rx_vlan_type",	"rx_vlant",	"rx VLAN type filter discards" },
#define	S_RX_VLAN_ID	AFTER(S_RX_VLAN_TYPE)
	{ 5,	"rx_vlan_id",	"rx_vlani",	"rx VLAN Id filter discards" },
#define	S_BADSRC	AFTER(S_RX_VLAN_ID)
	{ 5,	"badsrc",	"badsrc",	"rx invalid source discards" },
#define	S_BLACKLIST	AFTER(S_BADSRC)
	{ 5,	"blacklist",	"blacklist",	"rx black list discards" },
#define	S_WHITELIST	AFTER(S_BLACKLIST)
	{ 5,	"whitelist",	"whitelist",	"rx white list discards" },
#define	S_UNDERFLOW	AFTER(S_WHITELIST)
	{ 5,	"underflow",	"underflow",	"rx underflow entry discards" },
#define	S_COLL_SINGLE	AFTER(S_UNDERFLOW)
	{ 5,	"collision1",	"collision1",	"single collision frames" },
#define	S_COLL_MULTI	AFTER(S_COLL_SINGLE)
	{ 5,	"collisionM",	"collisionM",	"multiple collision frames" },
#define	S_DEFERRED	AFTER(S_COLL_MULTI)
	{ 5,	"deferred",	"deferred",	"deferred transmissions" },
#define	S_LATE		AFTER(S_DEFERRED)
	{ 5,	"late",		"late",		"late collisions" },
#define	S_EXCESSIVE	AFTER(S_LATE)
	{ 5,	"excessive",	"excessive",	"excessive collisions" },
#define	S_MACTX		AFTER(S_EXCESSIVE)
	{ 7,	"mactx",	"mactx",	"internal MAC tx errors" },
#define	S_CARRIER	AFTER(S_MACTX)
	{ 7,	"carrier",	"carrier",	"carrier sense errors" },
#define	S_TOOBIG	AFTER(S_CARRIER)
	{ 7,	"toobig",	"toobig",	"tx large frame discards" },
#define	S_TX_VLAN_ID	AFTER(S_TOOBIG)
	{ 7,	"tx_vlan_id",	"tx_vlani",	"tx VLAN Id filter discards" },
};
#define	S_LAST		S_TX_VLAN_ID

/*
 * Stat block returned by NPE with NPE_GETSTATS msg.
 */
struct npestats {
	uint32_t dot3StatsAlignmentErrors;
	uint32_t dot3StatsFCSErrors;
	uint32_t dot3StatsInternalMacReceiveErrors;
	uint32_t RxOverrunDiscards;
	uint32_t RxLearnedEntryDiscards;
	uint32_t RxLargeFramesDiscards;
	uint32_t RxSTPBlockedDiscards;
	uint32_t RxVLANTypeFilterDiscards;
	uint32_t RxVLANIdFilterDiscards;
	uint32_t RxInvalidSourceDiscards;
	uint32_t RxBlackListDiscards;
	uint32_t RxWhiteListDiscards;
	uint32_t RxUnderflowEntryDiscards;
	uint32_t dot3StatsSingleCollisionFrames;
	uint32_t dot3StatsMultipleCollisionFrames;
	uint32_t dot3StatsDeferredTransmissions;
	uint32_t dot3StatsLateCollisions;
	uint32_t dot3StatsExcessiveCollisions;
	uint32_t dot3StatsInternalMacTransmitErrors;
	uint32_t dot3StatsCarrierSenseErrors;
	uint32_t TxLargeFrameDiscards;
	uint32_t TxVLANIdFilterDiscards;
};

struct npestatfoo_p {
	struct npestatfoo base;
	char oid[80];
	int mib[4];
	struct npestats cur;
	struct npestats total;
};

static void
npe_setifname(struct npestatfoo *wf0, const char *ifname)
{
	struct npestatfoo_p *wf = (struct npestatfoo_p *) wf0;
	size_t len;

	snprintf(wf->oid, sizeof(wf->oid), "dev.npe.%s.stats", ifname+3);
	len = 4;
	if (sysctlnametomib(wf->oid, wf->mib, &len) < 0)
		err(1, "sysctlnametomib: %s", wf->oid);
}

static void
npe_collect(struct npestatfoo_p *wf, struct npestats *stats)
{
	size_t len = sizeof(struct npestats);
	if (sysctl(wf->mib, 4, stats, &len, NULL, 0) < 0)
		err(1, "sysctl: %s", wf->oid);
}

static void
npe_collect_cur(struct statfoo *sf)
{
	struct npestatfoo_p *wf = (struct npestatfoo_p *) sf;

	npe_collect(wf, &wf->cur);
}

static void
npe_collect_tot(struct statfoo *sf)
{
	struct npestatfoo_p *wf = (struct npestatfoo_p *) sf;

	npe_collect(wf, &wf->total);
}

static void
npe_update_tot(struct statfoo *sf)
{
	struct npestatfoo_p *wf = (struct npestatfoo_p *) sf;

	wf->total = wf->cur;
}

static int
npe_get_curstat(struct statfoo *sf, int s, char b[], size_t bs)
{
	struct npestatfoo_p *wf = (struct npestatfoo_p *) sf;
#define	STAT(x) \
	snprintf(b, bs, "%u", wf->cur.x - wf->total.x); return 1

	switch (s) {
	case S_ALIGN:		STAT(dot3StatsAlignmentErrors);
	case S_FCS:		STAT(dot3StatsFCSErrors);
	case S_MACRX:		STAT(dot3StatsInternalMacReceiveErrors);
	case S_RXORN:		STAT(RxOverrunDiscards);
	case S_LEARN:		STAT(RxLearnedEntryDiscards);
	case S_LARGE:		STAT(RxLargeFramesDiscards);
	case S_STP:		STAT(RxSTPBlockedDiscards);
	case S_RX_VLAN_TYPE:	STAT(RxVLANTypeFilterDiscards);
	case S_RX_VLAN_ID:	STAT(RxVLANIdFilterDiscards);
	case S_BADSRC:		STAT(RxInvalidSourceDiscards);
	case S_BLACKLIST:	STAT(RxBlackListDiscards);
	case S_WHITELIST:	STAT(RxWhiteListDiscards);
	case S_UNDERFLOW:	STAT(RxUnderflowEntryDiscards);
	case S_COLL_SINGLE:	STAT(dot3StatsSingleCollisionFrames);
	case S_COLL_MULTI:	STAT(dot3StatsMultipleCollisionFrames);
	case S_DEFERRED:	STAT(dot3StatsDeferredTransmissions);
	case S_LATE:		STAT(dot3StatsLateCollisions);
	case S_EXCESSIVE:	STAT(dot3StatsExcessiveCollisions);
	case S_MACTX:		STAT(dot3StatsInternalMacTransmitErrors);
	case S_CARRIER:		STAT(dot3StatsCarrierSenseErrors);
	case S_TOOBIG:		STAT(TxLargeFrameDiscards);
	case S_TX_VLAN_ID:	STAT(TxVLANIdFilterDiscards);
	}
	b[0] = '\0';
	return 0;
#undef STAT
}

static int
npe_get_totstat(struct statfoo *sf, int s, char b[], size_t bs)
{
	struct npestatfoo_p *wf = (struct npestatfoo_p *) sf;
#define	STAT(x) \
	snprintf(b, bs, "%u", wf->total.x); return 1

	switch (s) {
	case S_ALIGN:		STAT(dot3StatsAlignmentErrors);
	case S_FCS:		STAT(dot3StatsFCSErrors);
	case S_MACRX:		STAT(dot3StatsInternalMacReceiveErrors);
	case S_RXORN:		STAT(RxOverrunDiscards);
	case S_LEARN:		STAT(RxLearnedEntryDiscards);
	case S_LARGE:		STAT(RxLargeFramesDiscards);
	case S_STP:		STAT(RxSTPBlockedDiscards);
	case S_RX_VLAN_TYPE:	STAT(RxVLANTypeFilterDiscards);
	case S_RX_VLAN_ID:	STAT(RxVLANIdFilterDiscards);
	case S_BADSRC:		STAT(RxInvalidSourceDiscards);
	case S_BLACKLIST:	STAT(RxBlackListDiscards);
	case S_WHITELIST:	STAT(RxWhiteListDiscards);
	case S_UNDERFLOW:	STAT(RxUnderflowEntryDiscards);
	case S_COLL_SINGLE:	STAT(dot3StatsSingleCollisionFrames);
	case S_COLL_MULTI:	STAT(dot3StatsMultipleCollisionFrames);
	case S_DEFERRED:	STAT(dot3StatsDeferredTransmissions);
	case S_LATE:		STAT(dot3StatsLateCollisions);
	case S_EXCESSIVE:	STAT(dot3StatsExcessiveCollisions);
	case S_MACTX:		STAT(dot3StatsInternalMacTransmitErrors);
	case S_CARRIER:		STAT(dot3StatsCarrierSenseErrors);
	case S_TOOBIG:		STAT(TxLargeFrameDiscards);
	case S_TX_VLAN_ID:	STAT(TxVLANIdFilterDiscards);
	}
	b[0] = '\0';
	return 0;
#undef STAT
}

STATFOO_DEFINE_BOUNCE(npestatfoo)

struct npestatfoo *
npestats_new(const char *ifname, const char *fmtstring)
{
#define	N(a)	(sizeof(a) / sizeof(a[0]))
	struct npestatfoo_p *wf;

	wf = calloc(1, sizeof(struct npestatfoo_p));
	if (wf != NULL) {
		statfoo_init(&wf->base.base, "npestats", npestats, N(npestats));
		/* override base methods */
		wf->base.base.collect_cur = npe_collect_cur;
		wf->base.base.collect_tot = npe_collect_tot;
		wf->base.base.get_curstat = npe_get_curstat;
		wf->base.base.get_totstat = npe_get_totstat;
		wf->base.base.update_tot = npe_update_tot;

		/* setup bounce functions for public methods */
		STATFOO_BOUNCE(wf, npestatfoo);

		/* setup our public methods */
		wf->base.setifname = npe_setifname;

		npe_setifname(&wf->base, ifname);
		wf->base.setfmt(&wf->base, fmtstring);
	}
	return &wf->base;
#undef N
}