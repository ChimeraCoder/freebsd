
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <err.h>

#include "eeprom.h"
#include "9287.h"

void
usage(char *argv[])
{
	printf("Usage: %s <eeprom dump file>\n", argv[0]);
	printf("\n");
	printf("  The eeprom dump file is a text hexdump of an EEPROM.\n");
	printf("  The lines must be formatted as follows:\n");
	printf("  0xAAAA: 0xDD 0xDD 0xDD 0xDD 0xDD 0xDD 0xDD 0xDD\n");
	printf("  where each line must have exactly eight data bytes.\n");
	exit(127);
}

int
main(int argc, char *argv[])
{
	uint16_t *eep = NULL;
	eep = calloc(4096, sizeof(int16_t));

	if (argc < 2)
		usage(argv);

	load_eeprom_dump(argv[1], eep);

	eeprom_9287_base_print(eep);
	eeprom_9287_custdata_print(eep);
	printf("\n2.4ghz:\n");
	eeprom_9287_modal_print(eep);
	printf("\n");

	eeprom_9287_calfreqpiers_print(eep);
	printf("\n");

	eeprom_9287_print_targets(eep);
	printf("\n");

	eeprom_9287_ctl_print(eep);
	printf("\n");

	eeprom_9287_print_edges(eep);
	printf("\n");

	eeprom_9287_print_other(eep);
	printf("\n");

	free(eep);
	exit(0);
}