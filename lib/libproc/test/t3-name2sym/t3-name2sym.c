
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
#include <gelf.h>
#include <string.h>

int
main(int argc, char *argv[])
{
	prmap_t *map = NULL;
	struct proc_handle *phdl;
	GElf_Sym sym;

	proc_create("./t3-name2sym", argv, NULL, NULL, &phdl);
	memset(&sym, 0, sizeof(sym));
	assert(proc_name2sym(phdl, "ld-elf.so.1", "r_debug_state", &sym) == 0);
	printf("0x%lx\n", sym.st_value);
	assert(proc_name2sym(phdl, "t3-name2sym", "main", &sym) == 0);
	printf("0x%lx\n", sym.st_value);
}