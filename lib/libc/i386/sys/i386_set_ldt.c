
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

#include <sys/cdefs.h>
#include <machine/segments.h>
#include <machine/sysarch.h>

int
i386_set_ldt(int start, union descriptor *descs, int num)
{
	struct i386_ldt_args p;

	p.start = start;
	p.descs = descs;
	p.num   = num;

	return sysarch(I386_SET_LDT, &p);
}