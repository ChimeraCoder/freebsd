
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
#include <stdlib.h>

#ifndef __has_include
#define __has_include(inc) 0
#endif

#if __APPLE__
#include <cxxabi.h>
#elif defined(LIBCXXRT) || __has_include(<cxxabi.h>)
#include <cxxabi.h>
#endif

#include "typeinfo"

#if !(defined(_LIBCPPABI_VERSION) || defined(LIBCXXRT))

std::bad_cast::bad_cast() _NOEXCEPT
{
}

std::bad_cast::~bad_cast() _NOEXCEPT
{
}

const char*
std::bad_cast::what() const _NOEXCEPT
{
  return "std::bad_cast";
}

std::bad_typeid::bad_typeid() _NOEXCEPT
{
}

std::bad_typeid::~bad_typeid() _NOEXCEPT
{
}

const char*
std::bad_typeid::what() const _NOEXCEPT
{
  return "std::bad_typeid";
}

#if __APPLE__
  // On Darwin, the cxa_bad_* functions cannot be in the lower level library
  // because bad_cast and bad_typeid are defined in his higher level library
  void __cxxabiv1::__cxa_bad_typeid() { throw std::bad_typeid(); }
  void __cxxabiv1::__cxa_bad_cast() { throw std::bad_cast(); }
#endif

#endif  // _LIBCPPABI_VERSION