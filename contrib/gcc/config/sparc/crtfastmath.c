
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

#define FPRS_NS		(1 << 22)	/* Non-Standard fpu results */

static void __attribute__((constructor))
set_fast_math (void)
{
  unsigned int fsr;

  /* This works for the 64-bit case because, even if 32-bit ld/st of
     the fsr register modified the upper 32-bit, the only thing up there
     are the 3 other condition codes which are "do not care" at the time
     that this runs.  */

  __asm__("st %%fsr, %0"
	  : "=m" (fsr));

  fsr |= FPRS_NS;

  __asm__("ld %0, %%fsr"
	  : : "m" (fsr));
}