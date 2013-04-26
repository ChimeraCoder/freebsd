
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

#include "rand48.h"

extern unsigned short _rand48_seed[3];
extern unsigned short _rand48_mult[3];
extern unsigned short _rand48_add;

void
lcong48(unsigned short p[7])
{
	_rand48_seed[0] = p[0];
	_rand48_seed[1] = p[1];
	_rand48_seed[2] = p[2];
	_rand48_mult[0] = p[3];
	_rand48_mult[1] = p[4];
	_rand48_mult[2] = p[5];
	_rand48_add = p[6];
}