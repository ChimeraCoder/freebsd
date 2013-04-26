
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

#include "ar5211/ar5211.h"
#include "ar5211/ar5211reg.h"
#include "ar5211/ar5211desc.h"

/*
 * Notify Power Mgt is enabled in self-generated frames.
 * If requested, force chip awake.
 *
 * Returns A_OK if chip is awake or successfully forced awake.
 *
 * WARNING WARNING WARNING
 * There is a problem with the chip where sometimes it will not wake up.
 */
static HAL_BOOL
ar5211SetPowerModeAwake(struct ath_hal *ah, int setChip)
{
#define	POWER_UP_TIME	2000
	uint32_t val;
	int i;

	if (setChip) {
		OS_REG_RMW_FIELD(ah, AR_SCR, AR_SCR_SLE, AR_SCR_SLE_WAKE);
		OS_DELAY(10);	/* Give chip the chance to awake */

		for (i = POWER_UP_TIME / 200; i != 0; i--) {
			val = OS_REG_READ(ah, AR_PCICFG);
			if ((val & AR_PCICFG_SPWR_DN) == 0)
				break;
			OS_DELAY(200);
			OS_REG_RMW_FIELD(ah, AR_SCR, AR_SCR_SLE,
				AR_SCR_SLE_WAKE);
		}
		if (i == 0) {
#ifdef AH_DEBUG
			ath_hal_printf(ah, "%s: Failed to wakeup in %ums\n",
				__func__, POWER_UP_TIME/20);
#endif
			return AH_FALSE;
		}
	} 

	OS_REG_CLR_BIT(ah, AR_STA_ID1, AR_STA_ID1_PWR_SAV);
	return AH_TRUE;
#undef POWER_UP_TIME
}

/*
 * Notify Power Mgt is disabled in self-generated frames.
 * If requested, force chip to sleep.
 */
static void
ar5211SetPowerModeSleep(struct ath_hal *ah, int setChip)
{
	OS_REG_SET_BIT(ah, AR_STA_ID1, AR_STA_ID1_PWR_SAV);
	if (setChip)
		OS_REG_RMW_FIELD(ah, AR_SCR, AR_SCR_SLE, AR_SCR_SLE_SLP);
}

/*
 * Notify Power Management is enabled in self-generating
 * fames.  If request, set power mode of chip to
 * auto/normal.  Duration in units of 128us (1/8 TU).
 */
static void
ar5211SetPowerModeNetworkSleep(struct ath_hal *ah, int setChip)
{
	OS_REG_SET_BIT(ah, AR_STA_ID1, AR_STA_ID1_PWR_SAV);
	if (setChip)
		OS_REG_RMW_FIELD(ah, AR_SCR, AR_SCR_SLE, AR_SCR_SLE_NORM);
}

HAL_BOOL
ar5211SetPowerMode(struct ath_hal *ah, HAL_POWER_MODE mode, int setChip)
{
	struct ath_hal_5211 *ahp = AH5211(ah);
#ifdef AH_DEBUG
	static const char* modes[] = {
		"AWAKE",
		"FULL-SLEEP",
		"NETWORK SLEEP",
		"UNDEFINED"
	};
#endif
	int status = AH_TRUE;

	HALDEBUG(ah, HAL_DEBUG_POWER, "%s: %s -> %s (%s)\n", __func__,
		modes[ahp->ah_powerMode], modes[mode],
		setChip ? "set chip " : "");
	switch (mode) {
	case HAL_PM_AWAKE:
		status = ar5211SetPowerModeAwake(ah, setChip);
		break;
	case HAL_PM_FULL_SLEEP:
		ar5211SetPowerModeSleep(ah, setChip);
		break;
	case HAL_PM_NETWORK_SLEEP:
		ar5211SetPowerModeNetworkSleep(ah, setChip);
		break;
	default:
		HALDEBUG(ah, HAL_DEBUG_ANY, "%s: unknown power mode %u\n",
		    __func__, mode);
		return AH_FALSE;
	}
	ahp->ah_powerMode = mode;
	return status; 
}

HAL_POWER_MODE
ar5211GetPowerMode(struct ath_hal *ah)
{
	/* Just so happens the h/w maps directly to the abstracted value */
	return MS(OS_REG_READ(ah, AR_SCR), AR_SCR_SLE);
}