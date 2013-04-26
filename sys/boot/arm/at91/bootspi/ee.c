
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

#include "at91rm9200_lowlevel.h"
#include "at91rm9200.h"
#include "lib.h"
#include "ee.h"

/******************************* GLOBALS *************************************/


/*********************** PRIVATE FUNCTIONS/DATA ******************************/


/* Use a macro to calculate the TWI clock generator value to save code space. */
#define AT91C_TWSI_CLOCK	100000
#define TWSI_EEPROM_ADDRESS	0x40

#define TWI_CLK_BASE_DIV	((AT91C_MASTER_CLOCK/(4*AT91C_TWSI_CLOCK)) - 2)
#define SET_TWI_CLOCK	((0x00010000) | (TWI_CLK_BASE_DIV) | (TWI_CLK_BASE_DIV << 8))


/*************************** GLOBAL FUNCTIONS ********************************/


/*
 * .KB_C_FN_DEFINITION_START
 * void InitEEPROM(void)
 *  This global function initializes the EEPROM interface (TWI).  Intended
 * to be called a single time.
 * .KB_C_FN_DEFINITION_END
 */
void
EEInit(void)
{

	AT91PS_TWI twiPtr = (AT91PS_TWI)AT91C_BASE_TWI;

	AT91PS_PIO pPio = (AT91PS_PIO)AT91C_BASE_PIOA;
	AT91PS_PMC pPMC = (AT91PS_PMC)AT91C_BASE_PMC;

	pPio->PIO_ASR = AT91C_PIO_PA25 | AT91C_PIO_PA26;
	pPio->PIO_PDR = AT91C_PIO_PA25 | AT91C_PIO_PA26;

	pPio->PIO_MDDR = ~AT91C_PIO_PA25;
	pPio->PIO_MDER = AT91C_PIO_PA25;

	pPMC->PMC_PCER = 1u << AT91C_ID_TWI;

	twiPtr->TWI_IDR = 0xffffffffu;
	twiPtr->TWI_CR = AT91C_TWI_SWRST;
	twiPtr->TWI_CR = AT91C_TWI_MSEN | AT91C_TWI_SVDIS;

	twiPtr->TWI_CWGR = SET_TWI_CLOCK;
}

static inline unsigned
iicaddr(unsigned ee_off)
{
    return (TWSI_EEPROM_ADDRESS | ((ee_off >> 8) & 0x7));
}


/*
 * .KB_C_FN_DEFINITION_START
 * void ReadEEPROM(unsigned ee_addr, char *data_addr, unsigned size)
 *  This global function reads data from the eeprom at ee_addr storing data
 * to data_addr for size bytes.  Assume the TWI has been initialized.
 * This function does not utilize the page read mode to simplify the code.
 * .KB_C_FN_DEFINITION_END
 */
void
EERead(unsigned ee_off, char *data_addr, unsigned size)
{
	const AT91PS_TWI 	twiPtr = AT91C_BASE_TWI;
	unsigned int status;

	if ((ee_off & ~0xff) != ((ee_off + size) & ~0xff)) {
		printf("Crosses page boundary: 0x%x 0x%x\n", ee_off, size);
		return;
	}

	status = twiPtr->TWI_SR;
	status = twiPtr->TWI_RHR;
	twiPtr->TWI_MMR = (iicaddr(ee_off) << 16) | AT91C_TWI_IADRSZ_1_BYTE |
	    AT91C_TWI_MREAD;
	twiPtr->TWI_IADR = ee_off & 0xff;
	twiPtr->TWI_CR = AT91C_TWI_START;
	while (size-- > 1) {
		while (!(twiPtr->TWI_SR & AT91C_TWI_RXRDY))
			continue;
		*(data_addr++) = twiPtr->TWI_RHR;
	}
	twiPtr->TWI_CR = AT91C_TWI_STOP;
	status = twiPtr->TWI_SR;
	while (!(twiPtr->TWI_SR & AT91C_TWI_TXCOMP))
		continue;
	*data_addr = twiPtr->TWI_RHR;
}


/*
 * .KB_C_FN_DEFINITION_START
 * void WriteEEPROM(unsigned ee_off, char *data_addr, unsigned size)
 *  This global function writes data to the eeprom at ee_off using data
 * from data_addr for size bytes.  Assume the TWI has been initialized.
 * This function does not utilize the page write mode as the write time is
 * much greater than the time required to access the device for byte-write
 * functionality.  This allows the function to be much simpler.
 * .KB_C_FN_DEFINITION_END
 */
void
EEWrite(unsigned ee_off, const char *data_addr, unsigned size)
{
	const AT91PS_TWI 	twiPtr = AT91C_BASE_TWI;
	unsigned		status;
	char			test_data;

	while (size--) {
		// Set the TWI Master Mode Register
		twiPtr->TWI_MMR = (iicaddr(ee_off) << 16) |
		    AT91C_TWI_IADRSZ_1_BYTE;
		twiPtr->TWI_IADR = ee_off++;
		status = twiPtr->TWI_SR;

		// Load one data byte
		twiPtr->TWI_THR = *(data_addr++);
		twiPtr->TWI_CR = AT91C_TWI_START;
		while (!(twiPtr->TWI_SR & AT91C_TWI_TXRDY))
			continue;
		twiPtr->TWI_CR = AT91C_TWI_STOP;
		status = twiPtr->TWI_SR;
		while (!(twiPtr->TWI_SR & AT91C_TWI_TXCOMP))
			continue;

		// wait for write operation to complete, it is done once
		// we can read it back...
		EERead(ee_off, &test_data, 1);
	}
}