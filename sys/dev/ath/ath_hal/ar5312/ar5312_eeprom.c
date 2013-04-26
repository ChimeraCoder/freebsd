
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


#ifdef AH_SUPPORT_AR5312

#include "ah.h"
#include "ah_internal.h"

#include "ar5312/ar5312.h"
#include "ar5312/ar5312reg.h"
#include "ar5212/ar5212desc.h"

/*
 * Read 16 bits of data from offset into *data
 */
HAL_BOOL
ar5312EepromRead(struct ath_hal *ah, u_int off, uint16_t *dataIn)
{
        int i,offset;
	const char *eepromAddr = AR5312_RADIOCONFIG(ah);
	uint8_t *data;
	
	data = (uint8_t *) dataIn;
	for (i=0,offset=2*off; i<2; i++,offset++) {
		data[i] = eepromAddr[offset];
	}
        return AH_TRUE;
}
#endif /* AH_SUPPORT_AR5312 */