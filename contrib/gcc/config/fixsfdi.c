
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
typedef int DItype __attribute__ ((mode (DI)));
typedef unsigned int UDItype __attribute__ ((mode (DI)));
typedef float SFtype __attribute__ ((mode (SF)));
typedef float DFtype __attribute__ ((mode (DF)));

DItype __fixsfdi (SFtype);

/* This version is needed to prevent recursion; fixunssfdi in libgcc
   calls fixsfdi, which in turn calls calls fixunssfdi.  */

static DItype
local_fixunssfdi (SFtype original_a)
{
  DFtype a = original_a;
  USItype hi, lo;

  hi = a / (((UDItype) 1) << (sizeof (USItype) * 8));
  lo = a - ((DFtype) hi) * (((UDItype) 1) << (sizeof (USItype) * 8));
  return ((UDItype) hi << (sizeof (USItype) * 8)) | lo;
}

DItype
__fixsfdi (SFtype a)
{
  if (a < 0)
    return - local_fixunssfdi (-a);
  return local_fixunssfdi (a);
}