
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
#include "ah.h"
#include "ah_internal.h"

#include "ah_eeprom_v4k.h"

#include "ar9002/ar9285.h"
#include "ar5416/ar5416reg.h"
#include "ar5416/ar5416phy.h"
#include "ar9002/ar9002phy.h"
//#include "ar9002/ar9287phy.h"

#include "ar9002/ar9287_cal.h"


void
ar9287PACal(struct ath_hal *ah, HAL_BOOL is_reset)
{
	/* XXX not required */
}

/*
 * This is like Merlin but without ADC disable
 */
HAL_BOOL
ar9287InitCalHardware(struct ath_hal *ah, const struct ieee80211_channel *chan)
{
	OS_REG_SET_BIT(ah, AR_PHY_AGC_CONTROL, AR_PHY_AGC_CONTROL_FLTR_CAL);
	
	/* Calibrate the AGC */
	OS_REG_WRITE(ah, AR_PHY_AGC_CONTROL,
	    OS_REG_READ(ah, AR_PHY_AGC_CONTROL) | AR_PHY_AGC_CONTROL_CAL);

	/* Poll for offset calibration complete */
	if (!ath_hal_wait(ah, AR_PHY_AGC_CONTROL,
	    AR_PHY_AGC_CONTROL_CAL, 0)) {
		HALDEBUG(ah, HAL_DEBUG_RESET,
		    "%s: offset calibration failed to complete in 1ms; "
		    "noisy environment?\n", __func__);
		return AH_FALSE;
	}

	OS_REG_CLR_BIT(ah, AR_PHY_AGC_CONTROL, AR_PHY_AGC_CONTROL_FLTR_CAL);

	return AH_TRUE;
}