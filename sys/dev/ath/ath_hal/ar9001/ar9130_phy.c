
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

#include "ar5416/ar5416.h"
#include "ar5416/ar5416reg.h"
#include "ar5416/ar5416phy.h"
#include "ar9001/ar9130_phy.h"

void
ar9130InitPLL(struct ath_hal *ah, const struct ieee80211_channel *chan)
{

	uint32_t pll;

	/*
	 * XXX TODO: support half/quarter rates
	 */
	if (chan && IEEE80211_IS_CHAN_5GHZ(chan))
		pll = 0x1450;
	else
		pll = 0x1458;

	OS_REG_WRITE(ah, AR_RTC_PLL_CONTROL, pll);
	OS_DELAY(RTC_PLL_SETTLE_DELAY);
	OS_REG_WRITE(ah, AR_RTC_SLEEP_CLK, AR_RTC_SLEEP_DERIVED_CLK);
}