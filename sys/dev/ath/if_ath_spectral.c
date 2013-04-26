
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
 * Implement some basic spectral scan control logic.
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
#include <dev/ath/if_ath_spectral.h>

#include <dev/ath/ath_hal/ah_desc.h>

struct ath_spectral_state {
	HAL_SPECTRAL_PARAM	spectral_state;

	/*
	 * Should we enable spectral scan upon
	 * each network interface reset/change?
	 *
	 * This is intended to allow spectral scan
	 * frame reporting during channel scans.
	 *
	 * Later on it can morph into a larger
	 * scale config method where it pushes
	 * a "channel scan" config into the hardware
	 * rather than just the spectral_state
	 * config.
	 */
	int spectral_enable_after_reset;
};

/*
 * Methods which are required
 */

/*
 * Attach spectral to the given interface
 */
int
ath_spectral_attach(struct ath_softc *sc)
{
	struct ath_spectral_state *ss;

	/*
	 * If spectral isn't supported, don't error - just
	 * quietly complete.
	 */
	if (! ath_hal_spectral_supported(sc->sc_ah))
		return (0);

	ss = malloc(sizeof(struct ath_spectral_state),
	    M_TEMP, M_WAITOK | M_ZERO);

	if (ss == NULL) {
		device_printf(sc->sc_dev, "%s: failed to alloc memory\n",
		    __func__);
		return (-ENOMEM);
	}

	sc->sc_spectral = ss;

	(void) ath_hal_spectral_get_config(sc->sc_ah, &ss->spectral_state);

	return (0);
}

/*
 * Detach spectral from the given interface
 */
int
ath_spectral_detach(struct ath_softc *sc)
{

	if (! ath_hal_spectral_supported(sc->sc_ah))
		return (0);

	if (sc->sc_spectral != NULL) {
		free(sc->sc_spectral, M_TEMP);
	}
	return (0);
}

/*
 * Check whether spectral needs enabling and if so,
 * flip it on.
 */
int
ath_spectral_enable(struct ath_softc *sc, struct ieee80211_channel *ch)
{
	struct ath_spectral_state *ss = sc->sc_spectral;

	/* Default to disable spectral PHY reporting */
	sc->sc_dospectral = 0;

	if (ss == NULL)
		return (0);

	if (ss->spectral_enable_after_reset) {
		ath_hal_spectral_configure(sc->sc_ah,
		    &ss->spectral_state);
		(void) ath_hal_spectral_start(sc->sc_ah);
		sc->sc_dospectral = 1;
	}
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
ath_ioctl_spectral(struct ath_softc *sc, struct ath_diag *ad)
{
	unsigned int id = ad->ad_id & ATH_DIAG_ID;
	void *indata = NULL;
	void *outdata = NULL;
	u_int32_t insize = ad->ad_in_size;
	u_int32_t outsize = ad->ad_out_size;
	int error = 0;
	HAL_SPECTRAL_PARAM peout;
	HAL_SPECTRAL_PARAM *pe;
	struct ath_spectral_state *ss = sc->sc_spectral;
	int val;

	if (! ath_hal_spectral_supported(sc->sc_ah))
		return (EINVAL);

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
		case SPECTRAL_CONTROL_GET_PARAMS:
			memset(&peout, 0, sizeof(peout));
			outsize = sizeof(HAL_SPECTRAL_PARAM);
			ath_hal_spectral_get_config(sc->sc_ah, &peout);
			pe = (HAL_SPECTRAL_PARAM *) outdata;
			memcpy(pe, &peout, sizeof(*pe));
			break;
		case SPECTRAL_CONTROL_SET_PARAMS:
			if (insize < sizeof(HAL_SPECTRAL_PARAM)) {
				error = EINVAL;
				break;
			}
			pe = (HAL_SPECTRAL_PARAM *) indata;
			ath_hal_spectral_configure(sc->sc_ah, pe);
			/* Save a local copy of the updated parameters */
			ath_hal_spectral_get_config(sc->sc_ah,
			    &ss->spectral_state);
			break;
		case SPECTRAL_CONTROL_START:
			ath_hal_spectral_configure(sc->sc_ah,
			    &ss->spectral_state);
			(void) ath_hal_spectral_start(sc->sc_ah);
			sc->sc_dospectral = 1;
			/* XXX need to update the PHY mask in the driver */
			break;
		case SPECTRAL_CONTROL_STOP:
			(void) ath_hal_spectral_stop(sc->sc_ah);
			sc->sc_dospectral = 0;
			/* XXX need to update the PHY mask in the driver */
			break;
		case SPECTRAL_CONTROL_ENABLE_AT_RESET:
			if (insize < sizeof(int)) {
				device_printf(sc->sc_dev, "%d != %d\n",
				    insize,
				    (int) sizeof(int));
				error = EINVAL;
				break;
			}
			if (indata == NULL) {
				device_printf(sc->sc_dev, "indata=NULL\n");
				error = EINVAL;
				break;
			}
			val = * ((int *) indata);
			if (val == 0)
				ss->spectral_enable_after_reset = 0;
			else
				ss->spectral_enable_after_reset = 1;
			break;
		case SPECTRAL_CONTROL_ENABLE:
			/* XXX TODO */
		case SPECTRAL_CONTROL_DISABLE:
			/* XXX TODO */
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