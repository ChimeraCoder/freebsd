
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

#include <limits.h>
#include <math.h>

double
scalbln (double x, long n)
{
	int in;

	in = (int)n;
	if (in != n) {
		if (n > 0)
			in = INT_MAX;
		else
			in = INT_MIN;
	}
	return (scalbn(x, in));
}

float
scalblnf (float x, long n)
{
	int in;

	in = (int)n;
	if (in != n) {
		if (n > 0)
			in = INT_MAX;
		else
			in = INT_MIN;
	}
	return (scalbnf(x, in));
}

long double
scalblnl (long double x, long n)
{
	int in;

	in = (int)n;
	if (in != n) {
		if (n > 0)
			in = INT_MAX;
		else
			in = INT_MIN;
	}
	return (scalbnl(x, (int)n));
}