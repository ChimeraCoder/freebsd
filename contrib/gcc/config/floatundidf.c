
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
/* Public domain.  */typedef unsigned int USItype __attribute__ ((mode (SI)));
typedef unsigned int UDItype __attribute__ ((mode (DI)));
typedef float DFtype __attribute__ ((mode (DF)));

DFtype __floatundidf (UDItype);

DFtype
__floatundidf (UDItype u)
{
  /* When the word size is small, we never get any rounding error.  */
  DFtype f = (USItype) (u >> (sizeof (USItype) * 8));
  f *= 0x1p32f;
  f += (USItype) u;
  return f;
}