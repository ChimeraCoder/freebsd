
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

/******************************************************************************
 *
 * Filename: emac.c
 *
 * Instantiation of routines for MAC/ethernet functions supporting tftp.
 *
 * Revision information:
 *
 * 28AUG2004	kb_admin	initial creation
 * 08JAN2005	kb_admin	added tftp download
 *					also adapted from external sources
 *
 * BEGIN_KBDD_BLOCK
 * No warranty, expressed or implied, is included with this software.  It is
 * provided "AS IS" and no warranty of any kind including statutory or aspects
 * relating to merchantability or fitness for any purpose is provided.  All
 * intellectual property rights of others is maintained with the respective
 * owners.  This software is not copyrighted and is intended for reference
 * only.
 * END_BLOCK
 ******************************************************************************/

#include "at91rm9200.h"
#include "at91rm9200_lowlevel.h"
#include "emac.h"
#include "lib.h"

/* ****************************** GLOBALS *************************************/

unsigned localMACSet;
unsigned char localMACAddr[6];
unsigned localMAClow, localMAChigh;

/* ********************** PRIVATE FUNCTIONS/DATA ******************************/

/*
 * .KB_C_FN_DEFINITION_START
 * void EMAC_SetMACAddress(unsigned low_address, unsigned high_address)
 *  This global function sets the MAC address.  low_address is the first
 * four bytes while high_address is the last 2 bytes of the 48-bit value.
 * .KB_C_FN_DEFINITION_END
 */
void
EMAC_SetMACAddress(unsigned char mac[6])
{
	AT91PS_PMC	pPMC = AT91C_BASE_PMC;
	AT91PS_EMAC	pEmac = AT91C_BASE_EMAC;

	/* enable the peripheral clock before using EMAC */
	pPMC->PMC_PCER = ((unsigned) 1 << AT91C_ID_EMAC);

	memcpy(localMACAddr, mac, 6);
	localMAClow = (mac[3] << 24) | (mac[2] << 16) | (mac[1] << 8) | mac[0];
	localMAChigh = (mac[5] << 8) | mac[4];
	localMACSet = 1;

	AT91C_BASE_PMC->PMC_PCER = 1u << AT91C_ID_EMAC;
	AT91C_BASE_PIOA->PIO_ASR = 
	  AT91C_PIO_PA14 | AT91C_PIO_PA12 | AT91C_PIO_PA13 |
	  AT91C_PIO_PA8 | AT91C_PIO_PA16 | AT91C_PIO_PA9 |
	  AT91C_PIO_PA10 | AT91C_PIO_PA11 | AT91C_PIO_PA15 |
	  AT91C_PIO_PA7;
	AT91C_BASE_PIOA->PIO_PDR = 
	  AT91C_PIO_PA14 | AT91C_PIO_PA12 | AT91C_PIO_PA13 |
	  AT91C_PIO_PA8 | AT91C_PIO_PA16 | AT91C_PIO_PA9 |
	  AT91C_PIO_PA10 | AT91C_PIO_PA11 | AT91C_PIO_PA15 |
	  AT91C_PIO_PA7;
#if defined(BOOT_KB920X) | defined(BOOT_BWCT)	/* Really !RMII */
	AT91C_BASE_PIOB->PIO_BSR =
	  AT91C_PIO_PB12 | AT91C_PIO_PB13 | AT91C_PIO_PB14 |
	  AT91C_PIO_PB15 | AT91C_PIO_PB16 | AT91C_PIO_PB17 |
	  AT91C_PIO_PB18 | AT91C_PIO_PB19;
	AT91C_BASE_PIOB->PIO_PDR =
	  AT91C_PIO_PB12 | AT91C_PIO_PB13 | AT91C_PIO_PB14 |
	  AT91C_PIO_PB15 | AT91C_PIO_PB16 | AT91C_PIO_PB17 |
	  AT91C_PIO_PB18 | AT91C_PIO_PB19;
#endif
	pEmac->EMAC_CTL  = 0;

	pEmac->EMAC_CFG  = (pEmac->EMAC_CFG & ~(AT91C_EMAC_CLK)) |
#ifdef BOOT_TSC
	    AT91C_EMAC_RMII |
#endif
	    AT91C_EMAC_CLK_HCLK_32 | AT91C_EMAC_CAF;
	// the sequence write EMAC_SA1L and write EMAC_SA1H must be respected
	pEmac->EMAC_SA1L = localMAClow;
	pEmac->EMAC_SA1H = localMAChigh;
}