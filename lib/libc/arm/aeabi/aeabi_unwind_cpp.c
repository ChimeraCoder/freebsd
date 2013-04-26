
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

/*
 * Provide an implementation of __aeabi_unwind_cpp_pr{0,1,2}. These are
 * required by libc but are implemented in libgcc_eh.a which we don't link
 * against. The libgcc_eh.a version will be called so we call abort to
 * check this.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <stdlib.h>

void __aeabi_unwind_cpp_pr0(void) __hidden;
void __aeabi_unwind_cpp_pr1(void) __hidden;
void __aeabi_unwind_cpp_pr2(void) __hidden;

void
__aeabi_unwind_cpp_pr0(void)
{
	abort();
}

void
__aeabi_unwind_cpp_pr1(void)
{
	abort();
}

void
__aeabi_unwind_cpp_pr2(void)
{
	abort();
}