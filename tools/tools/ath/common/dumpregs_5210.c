
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
#include "diag.h"

#include "ah.h"
#include "ah_internal.h"
#include "ar5210/ar5210reg.h"
#include "ar5210/ar5210phy.h"

#include "dumpregs.h"

#define	N(a)	(sizeof(a) / sizeof(a[0]))

static struct dumpreg ar5210regs[] = {
    DEFBASIC(AR_TXDP0,		"TXDP0"),
    DEFBASIC(AR_TXDP1,		"TXDP1"),
    DEFBASICfmt(AR_CR,		"CR",		AR_CR_BITS),
    DEFBASIC(AR_RXDP,		"RXDP"),
    DEFBASICfmt(AR_CFG,		"CFG",		AR_CFG_BITS),
    /* NB: read clears pending interrupts */
    DEFVOIDfmt(AR_ISR,		"ISR",		AR_ISR_BITS),
    DEFBASICfmt(AR_IMR,		"IMR",		AR_IMR_BITS),
    DEFBASICfmt(AR_IER,		"IER",		AR_IER_BITS),
    DEFBASICfmt(AR_BCR,		"BCR",		AR_BCR_BITS),
    DEFBASICfmt(AR_BSR,		"BSR",		AR_BSR_BITS),
    DEFBASICfmt(AR_TXCFG,	"TXCFG",	AR_TXCFG_BITS),
    DEFBASIC(AR_RXCFG,		"RXCFG"),
    DEFBASIC(AR_MIBC,		"MIBC"),
    DEFBASIC(AR_TOPS,		"TOPS"),
    DEFBASIC(AR_RXNOFRM,	"RXNOFR"),
    DEFBASIC(AR_TXNOFRM,	"TXNOFR"),
    DEFBASIC(AR_RPGTO,		"RPGTO"),
    DEFBASIC(AR_RFCNT,		"RFCNT"),
    DEFBASIC(AR_MISC,		"MISC"),
    DEFBASICfmt(AR_RC,		"RC",		AR_RC_BITS),
    DEFBASICfmt(AR_SCR,		"SCR",		AR_SCR_BITS),
    DEFBASICfmt(AR_INTPEND,	"INTPEND",	AR_INTPEND_BITS),
    DEFBASIC(AR_SFR,		"SFR"),
    DEFBASICfmt(AR_PCICFG,	"PCICFG",	AR_PCICFG_BITS),
    DEFBASIC(AR_GPIOCR,		"GPIOCR"),
    DEFVOID(AR_GPIODO,		"GPIODO"),
    DEFVOID(AR_GPIODI,		"GPIODI"),
    DEFBASIC(AR_SREV,		"SREV"),
    DEFBASIC(AR_STA_ID0,	"STA_ID0"),
    DEFBASICfmt(AR_STA_ID1,	"STA_ID1",	AR_STA_ID1_BITS),
    DEFBASIC(AR_BSS_ID0,	"BSS_ID0"),
    DEFBASIC(AR_BSS_ID1,	"BSS_ID1"),
    DEFBASIC(AR_SLOT_TIME,	"SLOTTIME"),
    DEFBASIC(AR_TIME_OUT,	"TIME_OUT"),
    DEFBASIC(AR_RSSI_THR,	"RSSI_THR"),
    DEFBASIC(AR_RETRY_LMT,	"RETRY_LM"),
    DEFBASIC(AR_USEC,		"USEC"),
    DEFBASICfmt(AR_BEACON,		"BEACON",	AR_BEACON_BITS),
    DEFBASIC(AR_CFP_PERIOD,	"CFP_PER"),
    DEFBASIC(AR_TIMER0,		"TIMER0"),
    DEFBASIC(AR_TIMER1,		"TIMER1"),
    DEFBASIC(AR_TIMER2,		"TIMER2"),
    DEFBASIC(AR_TIMER3,		"TIMER3"),
    DEFBASIC(AR_IFS0,		"IFS0"),
    DEFBASIC(AR_IFS1,		"IFS1"	),
    DEFBASIC(AR_CFP_DUR,	"CFP_DUR"),
    DEFBASICfmt(AR_RX_FILTER,	"RXFILTER",	AR_BEACON_BITS),
    DEFBASIC(AR_MCAST_FIL0,	"MCAST_0"),
    DEFBASIC(AR_MCAST_FIL1,	"MCAST_1"),
    DEFBASIC(AR_TX_MASK0,	"TX_MASK0"),
    DEFBASIC(AR_TX_MASK1,	"TX_MASK1"),
    DEFVOID(AR_CLR_TMASK,	"CLR_TMASK"),
    DEFBASIC(AR_TRIG_LEV,	"TRIG_LEV"),
    DEFBASICfmt(AR_DIAG_SW,	"DIAG_SW",	AR_DIAG_SW_BITS),
    DEFBASIC(AR_TSF_L32,	"TSF_L32"),
    DEFBASIC(AR_TSF_U32,	"TSF_U32"),
    DEFBASIC(AR_LAST_TSTP,	"LAST_TST"),
    DEFBASIC(AR_RETRY_CNT,	"RETRYCNT"),
    DEFBASIC(AR_BACKOFF,	"BACKOFF"),
    DEFBASIC(AR_NAV,		"NAV"),
    DEFBASIC(AR_RTS_OK,		"RTS_OK"),
    DEFBASIC(AR_RTS_FAIL,	"RTS_FAIL"),
    DEFBASIC(AR_ACK_FAIL,	"ACK_FAIL"),
    DEFBASIC(AR_FCS_FAIL,	"FCS_FAIL"),
    DEFBASIC(AR_BEACON_CNT,	"BEAC_CNT"),

    DEFVOIDfmt(AR_PHY_FRCTL,	"PHY_FRCTL",	AR_PHY_FRCTL_BITS),
    DEFVOIDfmt(AR_PHY_AGC,	"PHY_AGC",	AR_PHY_AGC_BITS),
    DEFVOID(AR_PHY_CHIPID,	"PHY_CHIPID"),
    DEFVOIDfmt(AR_PHY_ACTIVE,	"PHY_ACTIVE",	AR_PHY_ACTIVE_BITS),
    DEFVOIDfmt(AR_PHY_AGCCTL,	"PHY_AGCCTL",	AR_PHY_AGCCTL_BITS),
};

static __constructor void
ar5210_ctor(void)
{
#define	MAC5210	SREV(1,0), SREV(2,0)
	register_regs(ar5210regs, N(ar5210regs), MAC5210, PHYANY);
	register_keycache(64, MAC5210, PHYANY);

	register_range(0x9800, 0x9840, DUMP_BASEBAND, MAC5210, PHYANY);
}