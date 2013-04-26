
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

void
load_eeprom_dump(const char *file, uint16_t *buf)
{
	unsigned int r[8];
	FILE *fp;
	char b[1024];
	int i;

	fp = fopen(file, "r");
	if (!fp)
		err(1, "fopen");

	while (!feof(fp)) {
		if (fgets(b, 1024, fp) == NULL)
			break;
		if (feof(fp))
			break;
		if (strlen(b) > 0)
			b[strlen(b)-1] = '\0';
		if (strlen(b) == 0)
			break;
		sscanf(b, "%x: %x %x %x %x %x %x %x %x\n",
		    &i, &r[0], &r[1], &r[2], &r[3], &r[4],
		    &r[5], &r[6], &r[7]);
		buf[i++] = r[0];
		buf[i++] = r[1];
		buf[i++] = r[2];
		buf[i++] = r[3];
		buf[i++] = r[4];
		buf[i++] = r[5];
		buf[i++] = r[6];
		buf[i++] = r[7];
	}
	fclose(fp);
}