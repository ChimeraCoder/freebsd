
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
	// setup GPIO	pPio->PIO_ASR = AT91C_PA17_TXD0 | AT91C_PA18_RXD0;
	pPio->PIO_PDR = AT91C_PA17_TXD0 | AT91C_PA18_RXD0;

	// enable power
	pPMC->PMC_PCER = 1u << AT91C_ID_US0;
}

static void
USART1_Init() {

	AT91PS_PIO pPio = (AT91PS_PIO)AT91C_BASE_PIOA;
	AT91PS_PMC pPMC = (AT91PS_PMC)AT91C_BASE_PMC;

	// setup GPIO
	pPio->PIO_ASR = AT91C_PB20_TXD1 | AT91C_PB21_RXD1;
	pPio->PIO_PDR = AT91C_PB20_TXD1 | AT91C_PB21_RXD1;

	// enable power
	pPMC->PMC_PCER = 1u << AT91C_ID_US1;
}

static void
USART2_Init() {

	AT91PS_PIO pPio = (AT91PS_PIO)AT91C_BASE_PIOA;
	AT91PS_PMC pPMC = (AT91PS_PMC)AT91C_BASE_PMC;

	// setup GPIO
	pPio->PIO_ASR = AT91C_PA23_TXD2 | AT91C_PA22_RXD2;
	pPio->PIO_PDR = AT91C_PA23_TXD2 | AT91C_PA22_RXD2;

	// enable power
	pPMC->PMC_PCER = 1u << AT91C_ID_US2;
}

static void
USART3_Init() {

	AT91PS_PIO pPio = (AT91PS_PIO)AT91C_BASE_PIOA;
	AT91PS_PMC pPMC = (AT91PS_PMC)AT91C_BASE_PMC;

	// setup GPIO
	pPio->PIO_BSR = AT91C_PA5_TXD3 | AT91C_PA6_RXD3;
	pPio->PIO_PDR = AT91C_PA5_TXD3 | AT91C_PA6_RXD3;

	// enable power
	pPMC->PMC_PCER = 1u << AT91C_ID_US3;
}

void
board_init(void)
{

	printf("\n\n");
	printf("BWCT FSB-A920-1\n");
	printf("http://www.bwct.de\n");
	printf("\n");
#if defined(SDRAM_128M)
	printf("AT92RM9200 180MHz 128MB\n");
#else
	printf("AT92RM9200 180MHz 64MB\n");
#endif
	printf("Initialising USART0\n");
	USART0_Init();
	printf("Initialising USART1\n");
	USART1_Init();
	printf("Initialising USART2\n");
	USART2_Init();
	printf("Initialising USART3\n");
	USART3_Init();
	printf("Initialising TWI\n");
	EEInit();
	printf("Initialising DS1672\n");
	DS1672_Init();
	printf("Initialising Ethernet\n");
	printf("MAC %x:%x:%x:%x:%x:%x\n", mac[0],
	    mac[1], mac[2], mac[3], mac[4], mac[5]);
	EMAC_Init();
	EMAC_SetMACAddress(mac);
	printf("Initialising SD-card\n");
	sdcard_init();
}

#include "../bootspi/ee.c"

int
drvread(void *buf, unsigned lba, unsigned nblk)
{
    return (MCI_read((char *)buf, lba << 9, nblk << 9));
}