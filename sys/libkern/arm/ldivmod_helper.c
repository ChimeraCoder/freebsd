
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

#ifdef __ARM_EABI__
#include <libkern/quad.h>

/*
 * Helper for __aeabi_ldivmod.
 * TODO: __divdi3 calls __qdivrem. We should do the same and use the
 * remainder value rather than re-calculating it.
 */
long long __kern_ldivmod(long long, long long, long long *);

long long
__kern_ldivmod(long long n, long long m, long long *rem)
{
	long long q;

	q = __divdi3(n, m);	/* q = n / m */
	*rem = n - m * q;

	return q;
}

#endif