
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
*/

#include <config.h>

int memcmp(const void *x, const void *y, size_t n);

int memcmp(const void *x, const void *y, size_t n)
{
	const uint8_t* x8 = (const uint8_t*)x;
	const uint8_t* y8 = (const uint8_t*)y;
	size_t i;
	for(i=0; i<n; i++) {
		if(x8[i] < y8[i])
			return -1;
		else if(x8[i] > y8[i])
			return 1;
	}
	return 0;
}