
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

/*
 * $FreeBSD$
 * This program just allocates as many pipes as it can to ensure
 * that using up all pipe memory doesn't cause a panic.
 */

int main (void)

{
	int i, returnval;
	int pipes[10000];
	for (i = 0; i < 10000; i++) {
		returnval = pipe(&pipes[i]);
	}
	printf("PASS\n");
}