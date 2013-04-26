
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
#include "at91rm9200_lowlevel.h"
#include "loader_prompt.h"
#include "emac.h"
#include "lib.h"

/*
 * .KB_C_FN_DEFINITION_START
 * int main(void)
 *  This global function waits at least one second, but not more than two 
 * seconds, for input from the serial port.  If no response is recognized,
 * it acts according to the parameters specified by the environment.  For 
 * example, the function might boot an operating system.  Do not return
 * from this function.
 * .KB_C_FN_DEFINITION_END
 */
int
main(void)
{
	InitEEPROM();
	EMAC_Init();
	LoadBootCommands();
	printf("\n\rKB9202(www.kwikbyte.com)\n\rAuto boot..\n\r");
	if (getc(1) == -1)
		ExecuteEnvironmentFunctions();
	Bootloader(0);

	return (1);
}