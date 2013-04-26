
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

/**
 * memory.cc - Contains stub definition of C++ new/delete operators.
 *
 * These definitions are intended to be used for testing and are weak symbols
 * to allow them to be replaced by definitions from a STL implementation.
 * These versions simply wrap malloc() and free(), they do not provide a
 * C++-specific allocator.
 */

#include <stddef.h>
#include <stdlib.h>
#include "stdexcept.h"
#include "atomic.h"


namespace std
{
	struct nothrow_t {};
}


/// The type of the function called when allocation fails.
typedef void (*new_handler)();
/**
 * The function to call when allocation fails.  By default, there is no
 * handler and a bad allocation exception is thrown if an allocation fails.
 */
static new_handler new_handl;

namespace std
{
	/**
	 * Sets a function to be called when there is a failure in new.
	 */
	__attribute__((weak))
	new_handler set_new_handler(new_handler handler)
	{
		return ATOMIC_SWAP(&new_handl, handler);
	}
	__attribute__((weak))
	new_handler get_new_handler(void)
	{
		return ATOMIC_LOAD(&new_handl);
	}
}


__attribute__((weak))
void* operator new(size_t size)
{
	if (0 == size)
	{
		size = 1;
	}
	void * mem = malloc(size);
	while (0 == mem)
	{
		new_handler h = std::get_new_handler();
		if (0 != h)
		{
			h();
		}
		else
		{
			throw std::bad_alloc();
		}
		mem = malloc(size);
	}

	return mem;
}

__attribute__((weak))
void* operator new(size_t size, const std::nothrow_t &) throw()
{
	if (0 == size)
	{
		size = 1;
	}
	void *mem = malloc(size);
	while (0 == mem)
	{
		new_handler h = std::get_new_handler();
		if (0 != h)
		{
			try
			{
				h();
			}
			catch (...)
			{
				// nothrow operator new should return NULL in case of
				// std::bad_alloc exception in new handler
				return NULL;
			}
		}
		else
		{
			return NULL;
		}
		mem = malloc(size);
	}

	return mem;
}


__attribute__((weak))
void operator delete(void * ptr)
{
	free(ptr);
}


__attribute__((weak))
void * operator new[](size_t size)
{
	return ::operator new(size);
}


__attribute__((weak))
void operator delete[](void * ptr) throw()
{
	::operator delete(ptr);
}