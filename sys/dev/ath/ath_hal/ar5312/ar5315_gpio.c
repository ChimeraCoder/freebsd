
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

#if (AH_SUPPORT_2316 || AH_SUPPORT_2317)

#include "ah.h"
#include "ah_internal.h"
#include "ah_devid.h"

#include "ar5312/ar5312.h"
#include "ar5312/ar5312reg.h"
#include "ar5312/ar5312phy.h"

#define	AR_NUM_GPIO	7		/* 6 GPIO pins */
#define	AR5315_GPIOD_MASK	0x0000007F	/* GPIO data reg r/w mask */

/*
 * Configure GPIO Output lines
 */
HAL_BOOL
ar5315GpioCfgOutput(struct ath_hal *ah, uint32_t gpio, HAL_GPIO_MUX_TYPE type)
{
	uint32_t gpioOffset = (AR5315_GPIO_BASE - ((uint32_t) ah->ah_sh));

	HALASSERT(gpio < AR_NUM_GPIO);

	OS_REG_WRITE(ah, gpioOffset+AR5315_GPIODIR,
		  (OS_REG_READ(ah, gpioOffset+AR5315_GPIODIR) &~ AR5315_GPIODIR_M(gpio))
		| AR5315_GPIODIR_O(gpio));

	return AH_TRUE;
}

/*
 * Configure GPIO Input lines
 */
HAL_BOOL
ar5315GpioCfgInput(struct ath_hal *ah, uint32_t gpio)
{
	uint32_t gpioOffset = (AR5315_GPIO_BASE - ((uint32_t) ah->ah_sh));

	HALASSERT(gpio < AR_NUM_GPIO);

	OS_REG_WRITE(ah, gpioOffset+AR5315_GPIODIR,
		  (OS_REG_READ(ah, gpioOffset+AR5315_GPIODIR) &~ AR5315_GPIODIR_M(gpio))
		| AR5315_GPIODIR_I(gpio));

	return AH_TRUE;
}

/*
 * Once configured for I/O - set output lines
 */
HAL_BOOL
ar5315GpioSet(struct ath_hal *ah, uint32_t gpio, uint32_t val)
{
	uint32_t reg;
        uint32_t gpioOffset = (AR5315_GPIO_BASE - ((uint32_t) ah->ah_sh));

	HALASSERT(gpio < AR_NUM_GPIO);

	reg =  OS_REG_READ(ah, gpioOffset+AR5315_GPIODO);
	reg &= ~(1 << gpio);
	reg |= (val&1) << gpio;

	OS_REG_WRITE(ah, gpioOffset+AR5315_GPIODO, reg);
	return AH_TRUE;
}

/*
 * Once configured for I/O - get input lines
 */
uint32_t
ar5315GpioGet(struct ath_hal *ah, uint32_t gpio)
{
	uint32_t gpioOffset = (AR5315_GPIO_BASE - ((uint32_t) ah->ah_sh));

	if (gpio < AR_NUM_GPIO) {
		uint32_t val = OS_REG_READ(ah, gpioOffset+AR5315_GPIODI);
		val = ((val & AR5315_GPIOD_MASK) >> gpio) & 0x1;
		return val;
	} else  {
		return 0xffffffff;
	}
}

/*
 * Set the GPIO Interrupt
 */
void
ar5315GpioSetIntr(struct ath_hal *ah, u_int gpio, uint32_t ilevel)
{
	uint32_t val;
	uint32_t gpioOffset = (AR5315_GPIO_BASE - ((uint32_t) ah->ah_sh));

	/* XXX bounds check gpio */
	val = OS_REG_READ(ah, gpioOffset+AR5315_GPIOINT);
	val &= ~(AR5315_GPIOINT_M | AR5315_GPIOINTLVL_M);
	val |= gpio << AR5315_GPIOINT_S;
	if (ilevel)
		val |= 2 << AR5315_GPIOINTLVL_S;	/* interrupt on pin high */
	else
		val |= 1 << AR5315_GPIOINTLVL_S;	/* interrupt on pin low */

	/* Don't need to change anything for low level interrupt. */
	OS_REG_WRITE(ah, gpioOffset+AR5315_GPIOINT, val);

	/* Change the interrupt mask. */
	(void) ar5212SetInterrupts(ah, AH5212(ah)->ah_maskReg | HAL_INT_GPIO);
}


#endif /* AH_SUPPORT_2316 || AH_SUPPORT_2317 */