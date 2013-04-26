
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
typedef int SItype __attribute__ ((mode (SI)));
typedef float SFtype __attribute__ ((mode (SF)));
typedef float DFtype __attribute__ ((mode (DF)));

USItype __fixunsdfsi (DFtype);

#define SItype_MIN \
  (- ((SItype) (((USItype) 1 << ((sizeof (SItype) * 8) - 1)) - 1)) - 1)

USItype
__fixunsdfsi (DFtype a)
{
  if (a >= - (DFtype) SItype_MIN)
    return (SItype) (a + SItype_MIN) - SItype_MIN;
  return (SItype) a;
}