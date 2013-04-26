
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
/* ffs -- Find the first bit set in the parameter
@deftypefn Supplemental int ffs (int @var{valu})

Find the first (least significant) bit set in @var{valu}.  Bits are
numbered from right to left, starting with bit 1 (corresponding to the
value 1).  If @var{valu} is zero, zero is returned.

@end deftypefn

*/

int
ffs (register int valu)
{
  register int bit;

  if (valu == 0)
    return 0;

  for (bit = 1; !(valu & 1); bit++)
  	valu >>= 1;

  return bit;
}