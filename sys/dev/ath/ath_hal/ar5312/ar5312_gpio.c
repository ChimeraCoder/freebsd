
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
#include "ah_devid.h"

#include "ar5312/ar5312.h"
#include "ar5312/ar5312reg.h"
#include "ar5312/ar5312phy.h"

#define	AR_NUM_GPIO	6		/* 6 GPIO pins */
#define	AR5312_GPIOD_MASK	0x0000002F	/* GPIO data reg r/w mask */

/*
 * Configure GPIO Output lines
 */
HAL_BOOL
ar5312GpioCfgOutput(struct ath_hal *ah, uint32_t gpio, HAL_GPIO_MUX_TYPE type)
{
	uint32_t gpioOffset = (AR5312_GPIO_BASE - ((uint32_t) ah->ah_sh));

	HALASSERT(gpio < AR_NUM_GPIO);

	OS_REG_WRITE(ah, gpioOffset+AR5312_GPIOCR,
		  (OS_REG_READ(ah, gpioOffset+AR5312_GPIOCR) &~ AR_GPIOCR_CR_A(gpio))
		| AR_GPIOCR_CR_A(gpio));

	return AH_TRUE;
}

/*
 * Configure GPIO Input lines
 */
HAL_BOOL
ar5312GpioCfgInput(struct ath_hal *ah, uint32_t gpio)
{
	uint32_t gpioOffset = (AR5312_GPIO_BASE - ((uint32_t) ah->ah_sh));

	HALASSERT(gpio < AR_NUM_GPIO);

	OS_REG_WRITE(ah, gpioOffset+AR5312_GPIOCR,
		  (OS_REG_READ(ah, gpioOffset+AR5312_GPIOCR) &~ AR_GPIOCR_CR_A(gpio))
		| AR_GPIOCR_CR_N(gpio));

	return AH_TRUE;
}

/*
 * Once configured for I/O - set output lines
 */
HAL_BOOL
ar5312GpioSet(struct ath_hal *ah, uint32_t gpio, uint32_t val)
{
	uint32_t reg;
        uint32_t gpioOffset = (AR5312_GPIO_BASE - ((uint32_t) ah->ah_sh));

	HALASSERT(gpio < AR_NUM_GPIO);

	reg =  OS_REG_READ(ah, gpioOffset+AR5312_GPIODO);
	reg &= ~(1 << gpio);
	reg |= (val&1) << gpio;

	OS_REG_WRITE(ah, gpioOffset+AR5312_GPIODO, reg);
	return AH_TRUE;
}

/*
 * Once configured for I/O - get input lines
 */
uint32_t
ar5312GpioGet(struct ath_hal *ah, uint32_t gpio)
{
	uint32_t gpioOffset = (AR5312_GPIO_BASE - ((uint32_t) ah->ah_sh));

	if (gpio < AR_NUM_GPIO) {
		uint32_t val = OS_REG_READ(ah, gpioOffset+AR5312_GPIODI);
		val = ((val & AR5312_GPIOD_MASK) >> gpio) & 0x1;
		return val;
	} else {
		return 0xffffffff;
	}
}

/*
 * Set the GPIO Interrupt
 */
void
ar5312GpioSetIntr(struct ath_hal *ah, u_int gpio, uint32_t ilevel)
{
	uint32_t val;
        uint32_t gpioOffset = (AR5312_GPIO_BASE - ((uint32_t) ah->ah_sh));

	/* XXX bounds check gpio */
	val = OS_REG_READ(ah, gpioOffset+AR5312_GPIOCR);
	val &= ~(AR_GPIOCR_CR_A(gpio) |
		 AR_GPIOCR_INT_MASK | AR_GPIOCR_INT_ENA | AR_GPIOCR_INT_SEL);
	val |= AR_GPIOCR_CR_N(gpio) | AR_GPIOCR_INT(gpio) | AR_GPIOCR_INT_ENA;
	if (ilevel)
		val |= AR_GPIOCR_INT_SELH;	/* interrupt on pin high */
	else
		val |= AR_GPIOCR_INT_SELL;	/* interrupt on pin low */

	/* Don't need to change anything for low level interrupt. */
	OS_REG_WRITE(ah, gpioOffset+AR5312_GPIOCR, val);

	/* Change the interrupt mask. */
	(void) ar5212SetInterrupts(ah, AH5212(ah)->ah_maskReg | HAL_INT_GPIO);
}


#endif /* AH_SUPPORT_AR5312 */