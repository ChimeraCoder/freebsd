
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
/* Public domain.  */typedef int DItype __attribute__ ((mode (DI)));
typedef unsigned int UDItype __attribute__ ((mode (DI)));
typedef unsigned int USItype __attribute__ ((mode (SI)));
typedef float SFtype __attribute__ ((mode (SF)));
typedef float DFtype __attribute__ ((mode (DF)));

SFtype __floatundisf (UDItype);

SFtype
__floatundisf (UDItype u)
{
  /* Protect against double-rounding error.
     Represent any low-order bits, that might be truncated by a bit that
     won't be lost.  The bit can go in anywhere below the rounding position
     of SFTYPE.  A fixed mask and bit position handles all usual
     configurations.  */
  if (53 < (sizeof (DItype) * 8)
      && 53 > ((sizeof (DItype) * 8) - 53 + 24))
    {
      if (u >= ((UDItype) 1 << 53))
	{
	  if ((UDItype) u & (((UDItype) 1 << (sizeof (DItype) * 8 - 53)) - 1))
	    {
	      u &= ~ (((UDItype) 1 << (sizeof (DItype) * 8 - 53)) - 1);
	      u |= (UDItype) 1 << (sizeof (DItype) * 8 - 53);
	    }
	}
    }
  /* Do the calculation in a wider type so that we don't lose any of
     the precision of the high word while multiplying it.  */
  DFtype f = (USItype) (u >> (sizeof (USItype) * 8));
  f *= 0x1p32f;
  f += (USItype) u;
  return (SFtype) f;
}