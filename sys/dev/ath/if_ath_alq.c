
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
#include "opt_ah.h"
#include "opt_ath.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/sysctl.h>
#include <sys/bus.h>
#include <sys/malloc.h>
#include <sys/proc.h>
#include <sys/pcpu.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/alq.h>
#include <sys/endian.h>
#include <sys/time.h>

#include <dev/ath/if_ath_alq.h>

#ifdef	ATH_DEBUG_ALQ
static struct ale *
if_ath_alq_get(struct if_ath_alq *alq, int len)
{
	struct ale *ale;

	if (alq->sc_alq_isactive == 0)
		return (NULL);

	ale = alq_getn(alq->sc_alq_alq, len, ALQ_NOWAIT);
	if (! ale)
		alq->sc_alq_numlost++;
	return (ale);
}

void
if_ath_alq_init(struct if_ath_alq *alq, const char *devname)
{

	bzero(alq, sizeof(*alq));

	strncpy(alq->sc_alq_devname, devname, ATH_ALQ_DEVNAME_LEN);
	printf("%s (%s): attached\n", __func__, alq->sc_alq_devname);
	snprintf(alq->sc_alq_filename, ATH_ALQ_FILENAME_LEN,
	    "/tmp/ath_%s_alq.log", alq->sc_alq_devname);

	/* XXX too conservative, right? */
	alq->sc_alq_qsize = (64*1024);
}

void
if_ath_alq_setcfg(struct if_ath_alq *alq, uint32_t macVer,
    uint32_t macRev, uint32_t phyRev, uint32_t halMagic)
{

	/* Store these in network order */
	alq->sc_alq_cfg.sc_mac_version = htobe32(macVer);
	alq->sc_alq_cfg.sc_mac_revision = htobe32(macRev);
	alq->sc_alq_cfg.sc_phy_rev = htobe32(phyRev);
	alq->sc_alq_cfg.sc_hal_magic = htobe32(halMagic);
}

void
if_ath_alq_tidyup(struct if_ath_alq *alq)
{

	if_ath_alq_stop(alq);
	printf("%s (%s): detached\n", __func__, alq->sc_alq_devname);
	bzero(alq, sizeof(*alq));
}

int
if_ath_alq_start(struct if_ath_alq *alq)
{
	int error;

	if (alq->sc_alq_isactive)
		return (0);

	/*
	 * Create a variable-length ALQ.
	 */
	error = alq_open(&alq->sc_alq_alq, alq->sc_alq_filename,
	    curthread->td_ucred, ALQ_DEFAULT_CMODE,
	    alq->sc_alq_qsize, 0);

	if (error != 0) {
		printf("%s (%s): failed, err=%d\n", __func__,
		    alq->sc_alq_devname, error);
	} else {
		printf("%s (%s): opened\n", __func__, alq->sc_alq_devname);
		alq->sc_alq_isactive = 1;
		if_ath_alq_post(alq, ATH_ALQ_INIT_STATE,
		    sizeof (struct if_ath_alq_init_state),
		    (char *) &alq->sc_alq_cfg);
	}
	return (error);
}

int
if_ath_alq_stop(struct if_ath_alq *alq)
{

	if (alq->sc_alq_isactive == 0)
		return (0);

	printf("%s (%s): closed\n", __func__, alq->sc_alq_devname);

	alq->sc_alq_isactive = 0;
	alq_close(alq->sc_alq_alq);
	alq->sc_alq_alq = NULL;

	return (0);
}

/*
 * Post a debug message to the ALQ.
 *
 * "len" is the size of the buf payload in bytes.
 */
void
if_ath_alq_post(struct if_ath_alq *alq, uint16_t op, uint16_t len,
    const char *buf)
{
	struct if_ath_alq_hdr *ap;
	struct ale *ale;
	struct timeval tv;

	if (! if_ath_alq_checkdebug(alq, op))
		return;

	microtime(&tv);

	/*
	 * Enforce some semblence of sanity on 'len'.
	 * Although strictly speaking, any length is possible -
	 * just be conservative so things don't get out of hand.
	 */
	if (len > ATH_ALQ_PAYLOAD_LEN)
		len = ATH_ALQ_PAYLOAD_LEN;

	ale = if_ath_alq_get(alq, len + sizeof(struct if_ath_alq_hdr));

	if (ale == NULL)
		return;

	ap = (struct if_ath_alq_hdr *) ale->ae_data;
	ap->threadid = htobe64((uint64_t) curthread->td_tid);
	ap->tstamp_sec = htobe32((uint32_t) tv.tv_sec);
	ap->tstamp_usec = htobe32((uint32_t) tv.tv_usec);
	ap->op = htobe16(op);
	ap->len = htobe16(len);

	/*
	 * Copy the payload _after_ the header field.
	 */
	memcpy(((char *) ap) + sizeof(struct if_ath_alq_hdr),
	    buf,
	    len);

	alq_post(alq->sc_alq_alq, ale);
}
#endif	/* ATH_DEBUG */