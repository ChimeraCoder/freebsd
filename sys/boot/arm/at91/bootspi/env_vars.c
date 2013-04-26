
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
#include "loader_prompt.h"
#include "lib.h"

/******************************* GLOBALS *************************************/
char	boot_commands[MAX_BOOT_COMMANDS][MAX_INPUT_SIZE];

char	env_table[MAX_ENV_SIZE_BYTES];

extern char	BootCommandSection;

/************************** PRIVATE FUNCTIONS ********************************/


static int	currentIndex;
static int	currentOffset;


/*
 * .KB_C_FN_DEFINITION_START
 * int ReadCharFromEnvironment(char *)
 *  This private function reads characters from the enviroment variables
 * to service the command prompt during auto-boot or just to setup the
 * default environment.  Returns positive value if valid character was
 * set in the pointer.  Returns negative value to signal input stream
 * terminated.  Returns 0 to indicate _wait_ condition.
 * .KB_C_FN_DEFINITION_END
 */
static int
ReadCharFromEnvironment(int timeout)
{
	int ch;

	if (currentIndex < MAX_BOOT_COMMANDS) {
		ch = boot_commands[currentIndex][currentOffset++];
		if (ch == '\0' || (currentOffset >= MAX_INPUT_SIZE)) {
			currentOffset = 0;
			++currentIndex;
			ch = '\r';
		}
		return (ch);
	}

	return (-1);
}


/*************************** GLOBAL FUNCTIONS ********************************/


/*
 * .KB_C_FN_DEFINITION_START
 * void DumpBootCommands(void)
 *  This global function displays the current boot commands.
 * .KB_C_FN_DEFINITION_END
 */
void
DumpBootCommands(void)
{
	int	i;

	for (i = 0; boot_commands[i][0]; i++)
		printf("0x%x : %s[E]\n", i, boot_commands[i]);
}


/*
 * .KB_C_FN_DEFINITION_START
 * void LoadBootCommands(void)
 *  This global function loads the existing boot commands from raw format and
 * coverts it to the standard, command-index format.  Notice, the processed
 * boot command table has much more space allocated than the actual table
 * stored in non-volatile memory.  This is because the processed table
 * exists in RAM which is larger than the non-volatile space.
 * .KB_C_FN_DEFINITION_END
 */
void
LoadBootCommands(void)
{
	int	index, j;
	char	*cptr;

	p_memset((char*)boot_commands, 0, sizeof(boot_commands));
	cptr = &BootCommandSection;
	for (index = 0; *cptr; index++) {
		for (j = 0; *cptr; j++)
			boot_commands[index][j] = *cptr++;
		cptr++;
	}
}


/*
 * .KB_C_FN_DEFINITION_START
 * void ExecuteEnvironmentFunctions(void)
 *  This global function executes applicable entries in the environment.
 * .KB_C_FN_DEFINITION_END
 */
void
ExecuteEnvironmentFunctions(void)
{
	currentIndex = 0;
	currentOffset = 0;

	DumpBootCommands();
	printf("Autoboot...\n");
	Bootloader(ReadCharFromEnvironment);
}