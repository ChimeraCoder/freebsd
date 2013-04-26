
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

#include <sys/types.h>
#include <sys/systm.h>

#ifdef __ARM_EABI__
/* We need to provide these functions never call them */
void __aeabi_unwind_cpp_pr0(void);
void __aeabi_unwind_cpp_pr1(void);
void __aeabi_unwind_cpp_pr2(void);

void
__aeabi_unwind_cpp_pr0(void)
{
	panic("__aeabi_unwind_cpp_pr0");
}

void
__aeabi_unwind_cpp_pr1(void)
{
	panic("__aeabi_unwind_cpp_pr1");
}

void
__aeabi_unwind_cpp_pr2(void)
{
	panic("__aeabi_unwind_cpp_pr2");
}
#endif