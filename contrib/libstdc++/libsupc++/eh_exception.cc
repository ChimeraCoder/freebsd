
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

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

#include "typeinfo"
#include "exception"
#include "unwind-cxx.h"

std::exception::~exception() throw() { }

std::bad_exception::~bad_exception() throw() { }

const char* 
std::exception::what() const throw()
{
  // NB: Another elegant option would be returning typeid(*this).name()
  // and not overriding what() in bad_exception, bad_alloc, etc.  In
  // that case, however, mangled names would be returned, PR 14493.
  return "std::exception";
}

const char* 
std::bad_exception::what() const throw()
{
  return "std::bad_exception";
}