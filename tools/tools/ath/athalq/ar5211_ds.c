
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/alq.h>
#include <sys/endian.h>

#include <dev/ath/if_ath_alq.h>
#include <dev/ath/ath_hal/ar5211/ar5211desc.h>

#include "ar5211_ds.h"

#define	MS(_v, _f)	( ((_v) & (_f)) >> _f##_S )
#define	MF(_v, _f) ( !! ((_v) & (_f)))

static void
ar5211_decode_txstatus(struct if_ath_alq_payload *a)
{
	struct ar5211_desc txs;

	/* XXX assumes txs is smaller than PAYLOAD_LEN! */
	memcpy(&txs, &a->payload, sizeof(struct ar5211_desc));

	printf("[%u.%06u] [%llu] TXSTATUS\n",
	    (unsigned int) be32toh(a->hdr.tstamp_sec),
	    (unsigned int) be32toh(a->hdr.tstamp_usec),
	    (unsigned long long) be64toh(a->hdr.threadid));

	/* ds_txstatus0 */
	printf("    Frmok=%d, xretries=%d, fifounderrun=%d, filt=%d\n",
	    MF(txs.ds_status0, AR_FrmXmitOK),
	    MF(txs.ds_status0, AR_ExcessiveRetries),
	    MF(txs.ds_status0, AR_FIFOUnderrun),
	    MF(txs.ds_status0, AR_Filtered));
	printf("    LongRetryCnt=%d, ShortRetryCnt=%d, VCollCnt=%d\n",
	    MS(txs.ds_status0, AR_LongRetryCnt),
	    MS(txs.ds_status0, AR_ShortRetryCnt),
	    MS(txs.ds_status0, AR_VirtCollCnt));
	printf("    SndTimestamp=0x%04x\n",
	    MS(txs.ds_status0, AR_SendTimestamp));

	/* ds_txstatus1 */
	printf("    Done=%d, SeqNum=0x%04x, AckRSSI=%d\n",
	    MF(txs.ds_status1, AR_Done),
	    MS(txs.ds_status1, AR_SeqNum),
	    MS(txs.ds_status1, AR_AckSigStrength));

	printf("\n ------\n");
}

static void
ar5211_decode_txdesc(struct if_ath_alq_payload *a)
{
	struct ar5211_desc txc;

	/* XXX assumes txs is smaller than PAYLOAD_LEN! */
	memcpy(&txc, &a->payload, sizeof(struct ar5211_desc));

	printf("[%u.%06u] [%llu] TXD\n",
	    (unsigned int) be32toh(a->hdr.tstamp_sec),
	    (unsigned int) be32toh(a->hdr.tstamp_usec),
	    (unsigned long long) be64toh(a->hdr.threadid));

	printf("  link=0x%08x, data=0x%08x\n",
	    txc.ds_link,
	    txc.ds_data);

	/* ds_ctl0 */
	printf("    Frame Len=%d\n", txc.ds_ctl0 & AR_FrameLen);
	printf("    TX Rate=0x%02x, RtsEna=%d, Veol=%d, ClrDstMask=%d AntModeXmit=0x%02x\n",
	    MS(txc.ds_ctl0, AR_XmitRate),
	    MF(txc.ds_ctl0, AR_RTSCTSEnable),
	    MF(txc.ds_ctl0, AR_VEOL),
	    MF(txc.ds_ctl0, AR_ClearDestMask),
	    MF(txc.ds_ctl0, AR_AntModeXmit));
	printf("    TxIntrReq=%d\n",
	    MF(txc.ds_ctl0, AR_TxInterReq));

	/* ds_ctl1 */
	printf("    BufLen=%d, TxMore=%d, EncryptKeyIdx=%d,FrType=0x%x\n",
	    txc.ds_ctl1 & AR_BufLen,
	    MF(txc.ds_ctl1, AR_More),
	    MS(txc.ds_ctl1, AR_EncryptKeyIdx),
	    MS(txc.ds_ctl1, AR_FrmType));
	printf("    NoAck=%d\n", MF(txc.ds_ctl1, AR_NoAck));

	printf("\n ------ \n");
}

static void
ar5211_decode_rxstatus(struct if_ath_alq_payload *a)
{
	struct ar5211_desc rxs;

	/* XXX assumes rxs is smaller than PAYLOAD_LEN! */
	memcpy(&rxs, &a->payload, sizeof(struct ar5211_desc));

	printf("[%u.%06u] [%llu] RXSTATUS\n",
	    (unsigned int) be32toh(a->hdr.tstamp_sec),
	    (unsigned int) be32toh(a->hdr.tstamp_usec),
	    (unsigned long long) be64toh(a->hdr.threadid));

	printf("  link=0x%08x, data=0x%08x\n",
	    rxs.ds_link,
	    rxs.ds_data);

	/* ds_rxstatus0 */
	printf("  DataLen=%d, ArMore=%d, RSSI=%d, RcvAntenna=0x%x\n",
	    rxs.ds_status0 & AR_DataLen,
	    MF(rxs.ds_status0, AR_More),
	    MS(rxs.ds_status0, AR_RcvSigStrength),
	    MS(rxs.ds_status0, AR_RcvAntenna));

	/* ds_rxstatus1 */
	printf("  RxDone=%d, RxFrameOk=%d, CrcErr=%d, DecryptCrcErr=%d\n",
	    MF(rxs.ds_status1, AR_Done),
	    MF(rxs.ds_status1, AR_FrmRcvOK),
	    MF(rxs.ds_status1, AR_CRCErr),
	    MF(rxs.ds_status1, AR_DecryptCRCErr));
	printf("  KeyIdxValid=%d\n",
	    MF(rxs.ds_status1, AR_KeyIdxValid));

	printf("  PhyErrCode=0x%02x\n",
	    MS(rxs.ds_status1, AR_PHYErr));

	printf("  KeyMiss=%d\n",
	    MF(rxs.ds_status1, AR_KeyCacheMiss));

	printf("  Timetamp: 0x%05x\n",
	    MS(rxs.ds_status1, AR_RcvTimestamp));

	printf("\n ------\n");
}

void
ar5211_alq_payload(struct if_ath_alq_payload *a)
{

		switch (be16toh(a->hdr.op)) {
			case ATH_ALQ_EDMA_TXSTATUS:	/* TXSTATUS */
				ar5211_decode_txstatus(a);
				break;
			case ATH_ALQ_EDMA_RXSTATUS:	/* RXSTATUS */
				ar5211_decode_rxstatus(a);
				break;
			case ATH_ALQ_EDMA_TXDESC:	/* TXDESC */
				ar5211_decode_txdesc(a);
				break;
			default:
				printf("[%d.%06d] [%lld] op: %d; len %d\n",
				    be32toh(a->hdr.tstamp_sec),
				    be32toh(a->hdr.tstamp_usec),
				    be64toh(a->hdr.threadid),
				    be16toh(a->hdr.op), be16toh(a->hdr.len));
		}
}