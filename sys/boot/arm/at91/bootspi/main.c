
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

#include "env_vars.h"
#include "at91rm9200.h"
#include "at91rm9200_lowlevel.h"
#include "loader_prompt.h"
#include "emac.h"
#include "lib.h"
#include "spi_flash.h"
#include "ee.h"

int
main(void)
{
	printf("\nBoot\n");
	EEInit();
	SPI_InitFlash();
#ifdef TSC_FPGA
	fpga_load();
#endif
	EMAC_Init();
	LoadBootCommands();
	if (getc(1) == -1) {
		start_wdog(30);
		ExecuteEnvironmentFunctions();
	}
	Bootloader(getc);
	return (1);
}