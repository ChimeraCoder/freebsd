
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

#include <math.h>

float
roundf(float x)
{
	float t;

	if (!isfinite(x))
		return (x);

	if (x >= 0.0) {
		t = floorf(x);
		if (t - x <= -0.5)
			t += 1.0;
		return (t);
	} else {
		t = floorf(-x);
		if (t + x <= -0.5)
			t += 1.0;
		return (-t);
	}
}