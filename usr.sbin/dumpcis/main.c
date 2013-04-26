
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "readcis.h"

static void
scanfile(char *name)
{
	int     fd;
	struct tuple_list *tl;

	fd = open(name, O_RDONLY);
	if (fd < 0)
		return;
	tl = readcis(fd);
	if (tl) {
		printf("Configuration data for file %s\n",
		    name);
		dumpcis(tl);
		freecis(tl);
	}
	close(fd);
}

int
main(int argc, char **argv)
{
	for (argc--, argv++; argc; argc--, argv++)
		scanfile(*argv);
	return 0;
}