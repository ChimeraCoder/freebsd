
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
#include "ah_devid.h"
#include "ah_eeprom_v4k.h"

#include "ar9002/ar9280.h"
#include "ar9002/ar9285.h"
#include "ar5416/ar5416reg.h"
#include "ar5416/ar5416phy.h"
#include "ar9002/ar9285phy.h"
#include "ar9002/ar9285_phy.h"

void
ar9285_antdiv_comb_conf_get(struct ath_hal *ah,
    struct ar9285_antcomb_conf *antconf)
{
	uint32_t regval;

	regval = OS_REG_READ(ah, AR_PHY_MULTICHAIN_GAIN_CTL);
	antconf->main_lna_conf = (regval & AR_PHY_9285_ANT_DIV_MAIN_LNACONF) >>
				  AR_PHY_9285_ANT_DIV_MAIN_LNACONF_S;
	antconf->alt_lna_conf = (regval & AR_PHY_9285_ANT_DIV_ALT_LNACONF) >>
				 AR_PHY_9285_ANT_DIV_ALT_LNACONF_S;
	antconf->fast_div_bias = (regval & AR_PHY_9285_FAST_DIV_BIAS) >>
				  AR_PHY_9285_FAST_DIV_BIAS_S;
}

void
ar9285_antdiv_comb_conf_set(struct ath_hal *ah,
    struct ar9285_antcomb_conf *antconf)
{
	uint32_t regval;

	regval = OS_REG_READ(ah, AR_PHY_MULTICHAIN_GAIN_CTL);
	regval &= ~(AR_PHY_9285_ANT_DIV_MAIN_LNACONF |
		    AR_PHY_9285_ANT_DIV_ALT_LNACONF |
		    AR_PHY_9285_FAST_DIV_BIAS);
	regval |= ((antconf->main_lna_conf << AR_PHY_9285_ANT_DIV_MAIN_LNACONF_S)
		   & AR_PHY_9285_ANT_DIV_MAIN_LNACONF);
	regval |= ((antconf->alt_lna_conf << AR_PHY_9285_ANT_DIV_ALT_LNACONF_S)
		   & AR_PHY_9285_ANT_DIV_ALT_LNACONF);
	regval |= ((antconf->fast_div_bias << AR_PHY_9285_FAST_DIV_BIAS_S)
		   & AR_PHY_9285_FAST_DIV_BIAS);

	OS_REG_WRITE(ah, AR_PHY_MULTICHAIN_GAIN_CTL, regval);
}

/*
 * Check whether combined + fast antenna diversity should be enabled.
 *
 * This enables software-driven RX antenna diversity based on RX
 * RSSI + antenna config packet sampling.
 */
HAL_BOOL
ar9285_check_div_comb(struct ath_hal *ah)
{
	uint8_t ant_div_ctl1;
	HAL_EEPROM_v4k *ee = AH_PRIVATE(ah)->ah_eeprom;
        const MODAL_EEP4K_HEADER *pModal = &ee->ee_base.modalHeader;

	/* For now, simply disable this until it's better debugged. -adrian */
	return AH_FALSE;

	if (! AR_SREV_KITE(ah))
		return AH_FALSE;

	if (pModal->version < 3)
		return AH_FALSE;

	ant_div_ctl1 = pModal->antdiv_ctl1;
	if ((ant_div_ctl1 & 0x1) && ((ant_div_ctl1 >> 3) & 0x1))
		return AH_TRUE;

	return AH_FALSE;
}