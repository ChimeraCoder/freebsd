
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

#include	<sys/select.h>

#include	<err.h>
#include	<stdio.h>
#include	<unistd.h>

/*
 * Check that pipes can be selected for writing in the reverse direction.
 */
int
main(int argc, char *argv[])
{
	int pip[2];
	fd_set set;
	int n;

	if (pipe(pip) == -1)
		err(1, "FAIL: pipe");

	FD_ZERO(&set);
	FD_SET(pip[0], &set);
	n = select(pip[1] + 1, NULL, &set, NULL, &(struct timeval){ 0, 0 });
	if (n != 1)
		errx(1, "FAIL: select initial reverse direction");

	n = write(pip[0], "x", 1);
	if (n != 1)
		err(1, "FAIL: write reverse direction");

	FD_ZERO(&set);
	FD_SET(pip[0], &set);
	n = select(pip[1] + 1, NULL, &set, NULL, &(struct timeval){ 0, 0 });
	if (n != 1)
		errx(1, "FAIL: select reverse direction after write");

	printf("PASS\n");

	return (0);
}