
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

#include <sys/types.h>
#include <assert.h>
#include <stdio.h>
#include <libproc.h>

int
main(int argc, char *argv[])
{
	prmap_t *map = NULL;
	struct proc_handle *phdl;

	proc_create("./t2-name2map", argv, NULL, NULL, &phdl);
	map = proc_name2map(phdl, "ld-elf.so.1");
	assert(map);
}