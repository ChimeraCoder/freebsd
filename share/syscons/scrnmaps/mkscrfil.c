
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

#include <sys/ioctl.h>
#include <sys/consio.h>
#include <stdio.h>

#include FIL

int main(int argc, char **argv)
{
	FILE *fd;

	if (argc == 2) {
		if ((fd = fopen(argv[1], "w")) == NULL) {
			perror(argv[1]);
			return 1;
		}
		fwrite(&scrmap, sizeof(scrmap_t), 1, fd);
		fclose(fd);
		return 0;
	}
	else {
		fprintf(stderr, "usage: %s <mapfile>\n", argv[0]);
		return 1;
	}
}