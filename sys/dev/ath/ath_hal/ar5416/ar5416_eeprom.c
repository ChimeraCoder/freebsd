
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

#include "ah_eeprom_v14.h"

#include "ar5416/ar5416.h"
#include "ar5416/ar5416reg.h"
#include "ar5416/ar5416phy.h"

/*
 * Read 16 bits of data from offset into *data
 */
HAL_BOOL
ar5416EepromRead(struct ath_hal *ah, u_int off, uint16_t *data)
{
        OS_REG_READ(ah,  AR5416_EEPROM_OFFSET + (off << AR5416_EEPROM_S));
       	if (!ath_hal_wait(ah, AR_EEPROM_STATUS_DATA,
	    AR_EEPROM_STATUS_DATA_BUSY | AR_EEPROM_STATUS_DATA_PROT_ACCESS, 0))
		return AH_FALSE;
       	*data = MS(OS_REG_READ(ah, AR_EEPROM_STATUS_DATA),
		   AR_EEPROM_STATUS_DATA_VAL);
	return AH_TRUE;
}