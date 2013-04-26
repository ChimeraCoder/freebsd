
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

#include <bits/c++config.h>
#include <cxxabi.h>
#include "unwind-cxx.h"

#if _GLIBCXX_HOSTED
#ifdef _GLIBCXX_HAVE_UNISTD_H
# include <unistd.h>
# define writestr(str)	write(2, str, sizeof(str) - 1)
# ifdef __GNU_LIBRARY__
  /* Avoid forcing the library's meaning of `write' on the user program
     by using the "internal" name (for use within the library).  */
/*#  define write(fd, buf, n)	__write((fd), (buf), (n))*/
# endif
#else
# include <cstdio>
# define writestr(str)	std::fputs(str, stderr)
#endif
#else
# define writestr(str) /* Empty */
#endif

extern "C" void
__cxxabiv1::__cxa_pure_virtual (void)
{
  writestr ("pure virtual method called\n");
  std::terminate ();
}