
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

#include "ar5416/ar5416.h"

static const int keyType[] = {
	1,	/* HAL_CIPHER_WEP */
	0,	/* HAL_CIPHER_AES_OCB */
	2,	/* HAL_CIPHER_AES_CCM */
	0,	/* HAL_CIPHER_CKIP */
	3,	/* HAL_CIPHER_TKIP */
	0,	/* HAL_CIPHER_CLR */
};

/*
 * Clear the specified key cache entry and any associated MIC entry.
 */
HAL_BOOL
ar5416ResetKeyCacheEntry(struct ath_hal *ah, uint16_t entry)
{
	struct ath_hal_5416 *ahp = AH5416(ah);

	if (ar5212ResetKeyCacheEntry(ah, entry)) {
		ahp->ah_keytype[entry] = keyType[HAL_CIPHER_CLR];
		return AH_TRUE;
	} else
		return AH_FALSE;
}

/*
 * Sets the contents of the specified key cache entry
 * and any associated MIC entry.
 */
HAL_BOOL
ar5416SetKeyCacheEntry(struct ath_hal *ah, uint16_t entry,
                       const HAL_KEYVAL *k, const uint8_t *mac,
                       int xorKey)
{
	struct ath_hal_5416 *ahp = AH5416(ah);

	if (ar5212SetKeyCacheEntry(ah, entry, k, mac, xorKey)) {
		ahp->ah_keytype[entry] = keyType[k->kv_type];
		return AH_TRUE;
	} else
		return AH_FALSE;
}