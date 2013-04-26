
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
#include <exception_defines.h>
#include "new"

using std::new_handler;
using std::bad_alloc;

extern "C" void *malloc (std::size_t);
extern new_handler __new_handler;

_GLIBCXX_WEAK_DEFINITION void *
operator new (std::size_t sz, const std::nothrow_t&) throw()
{
  void *p;

  /* malloc (0) is unpredictable; avoid it.  */
  if (sz == 0)
    sz = 1;
  p = (void *) malloc (sz);
  while (p == 0)
    {
      new_handler handler = __new_handler;
      if (! handler)
	return 0;
      try
	{
	  handler ();
	}
      catch (bad_alloc &)
	{
	  return 0;
	}

      p = (void *) malloc (sz);
    }

  return p;
}