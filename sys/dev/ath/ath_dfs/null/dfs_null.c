
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

/*
 * This implements an empty DFS module.
 */
#include "opt_ath.h"
#include "opt_inet.h"
#include "opt_wlan.h"

#include <sys/param.h>
#include <sys/systm.h> 
#include <sys/sysctl.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/errno.h>

#include <machine/bus.h>
#include <machine/resource.h>
#include <sys/bus.h>

#include <sys/socket.h>
 
#include <net/if.h>
#include <net/if_media.h>
#include <net/if_arp.h>
#include <net/ethernet.h>		/* XXX for ether_sprintf */

#include <net80211/ieee80211_var.h>

#include <net/bpf.h>

#ifdef INET
#include <netinet/in.h>
#include <netinet/if_ether.h>
#endif

#include <dev/ath/if_athvar.h>
#include <dev/ath/if_athdfs.h>

#include <dev/ath/ath_hal/ah_desc.h>

/*
 * Methods which are required
 */

/*
 * Attach DFS to the given interface
 */
int
ath_dfs_attach(struct ath_softc *sc)
{
	return (1);
}

/*
 * Detach DFS from the given interface
 */
int
ath_dfs_detach(struct ath_softc *sc)
{
	return (1);
}

/*
 * Enable radar check.  Return 1 if the driver should
 * enable radar PHY errors, or 0 if not.
 */
int
ath_dfs_radar_enable(struct ath_softc *sc, struct ieee80211_channel *chan)
{
#if 0
	HAL_PHYERR_PARAM pe;

	/* Check if the hardware supports radar reporting */
	/* XXX TODO: migrate HAL_CAP_RADAR/HAL_CAP_AR to somewhere public! */
	if (ath_hal_getcapability(sc->sc_ah,
	    HAL_CAP_PHYDIAG, 0, NULL) != HAL_OK)
		return (0);

	/* Check if the current channel is radar-enabled */
	if (! IEEE80211_IS_CHAN_DFS(chan))
		return (0);

	/* Fetch the default parameters */
	memset(&pe, '\0', sizeof(pe));
	if (! ath_hal_getdfsdefaultthresh(sc->sc_ah, &pe))
		return (0);

	/* Enable radar PHY error reporting */
	sc->sc_dodfs = 1;

	/* Tell the hardware to enable radar reporting */
	pe.pe_enabled = 1;

	/* Flip on extension channel events only if doing HT40 */
	if (IEEE80211_IS_CHAN_HT40(chan))
		pe.pe_extchannel = 1;
	else
		pe.pe_extchannel = 0;

	ath_hal_enabledfs(sc->sc_ah, &pe);

	/*
	 * Disable strong signal fast diversity - needed for
	 * AR5212 and similar PHYs for reliable short pulse
	 * duration.
	 */
	(void) ath_hal_setcapability(sc->sc_ah, HAL_CAP_DIVERSITY, 2, 0, NULL);

	return (1);
#else
	return (0);
#endif
}

/*
 * Explicity disable radar reporting.
 *
 * Return 0 if it was disabled, < 0 on error.
 */
int
ath_dfs_radar_disable(struct ath_softc *sc)
{
#if 0
	HAL_PHYERR_PARAM pe;

	(void) ath_hal_getdfsthresh(sc->sc_ah, &pe);
	pe.pe_enabled = 0;
	(void) ath_hal_enabledfs(sc->sc_ah, &pe);
	return (0);
#else
	return (0);
#endif
}

/*
 * Process DFS related PHY errors
 *
 * The mbuf is not "ours" and if we want a copy, we have
 * to take a copy.  It'll be freed after this function returns.
 */
void
ath_dfs_process_phy_err(struct ath_softc *sc, struct mbuf *m,
    uint64_t tsf, struct ath_rx_status *rxstat)
{

}

/*
 * Process the radar events and determine whether a DFS event has occured.
 *
 * This is designed to run outside of the RX processing path.
 * The RX path will call ath_dfs_tasklet_needed() to see whether
 * the task/callback running this routine needs to be called.
 */
int
ath_dfs_process_radar_event(struct ath_softc *sc,
    struct ieee80211_channel *chan)
{
	return (0);
}

/*
 * Determine whether the DFS check task needs to be queued.
 *
 * This is called in the RX task when the current batch of packets
 * have been received. It will return whether there are any radar
 * events for ath_dfs_process_radar_event() to handle.
 */
int
ath_dfs_tasklet_needed(struct ath_softc *sc, struct ieee80211_channel *chan)
{
	return (0);
}

/*
 * Handle ioctl requests from the diagnostic interface.
 *
 * The initial part of this code resembles ath_ioctl_diag();
 * it's likely a good idea to reduce duplication between
 * these two routines.
 */
int
ath_ioctl_phyerr(struct ath_softc *sc, struct ath_diag *ad)
{
	unsigned int id = ad->ad_id & ATH_DIAG_ID;
	void *indata = NULL;
	void *outdata = NULL;
	u_int32_t insize = ad->ad_in_size;
	u_int32_t outsize = ad->ad_out_size;
	int error = 0;
	HAL_PHYERR_PARAM peout;
	HAL_PHYERR_PARAM *pe;

	if (ad->ad_id & ATH_DIAG_IN) {
		/*
		 * Copy in data.
		 */
		indata = malloc(insize, M_TEMP, M_NOWAIT);
		if (indata == NULL) {
			error = ENOMEM;
			goto bad;
		}
		error = copyin(ad->ad_in_data, indata, insize);
		if (error)
			goto bad;
	}
	if (ad->ad_id & ATH_DIAG_DYN) {
		/*
		 * Allocate a buffer for the results (otherwise the HAL
		 * returns a pointer to a buffer where we can read the
		 * results).  Note that we depend on the HAL leaving this
		 * pointer for us to use below in reclaiming the buffer;
		 * may want to be more defensive.
		 */
		outdata = malloc(outsize, M_TEMP, M_NOWAIT);
		if (outdata == NULL) {
			error = ENOMEM;
			goto bad;
		}
	}
	switch (id) {
		case DFS_SET_THRESH:
			if (insize < sizeof(HAL_PHYERR_PARAM)) {
				error = EINVAL;
				break;
			}
			pe = (HAL_PHYERR_PARAM *) indata;
			ath_hal_enabledfs(sc->sc_ah, pe);
			break;
		case DFS_GET_THRESH:
			memset(&peout, 0, sizeof(peout));
			outsize = sizeof(HAL_PHYERR_PARAM);
			ath_hal_getdfsthresh(sc->sc_ah, &peout);
			pe = (HAL_PHYERR_PARAM *) outdata;
			memcpy(pe, &peout, sizeof(*pe));
			break;
		default:
			error = EINVAL;
	}
	if (outsize < ad->ad_out_size)
		ad->ad_out_size = outsize;
	if (outdata && copyout(outdata, ad->ad_out_data, ad->ad_out_size))
		error = EFAULT;
bad:
	if ((ad->ad_id & ATH_DIAG_IN) && indata != NULL)
		free(indata, M_TEMP);
	if ((ad->ad_id & ATH_DIAG_DYN) && outdata != NULL)
		free(outdata, M_TEMP);
	return (error);
}

/*
 * Get the current DFS thresholds from the HAL
 */
int
ath_dfs_get_thresholds(struct ath_softc *sc, HAL_PHYERR_PARAM *param)
{
	ath_hal_getdfsthresh(sc->sc_ah, param);
	return (1);
}