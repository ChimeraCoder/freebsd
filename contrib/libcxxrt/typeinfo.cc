
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

#include "typeinfo.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

using std::type_info;

type_info::~type_info() {}

bool type_info::operator==(const type_info &other) const
{
	return __type_name == other.__type_name;
}
bool type_info::operator!=(const type_info &other) const
{
	return __type_name != other.__type_name;
}
bool type_info::before(const type_info &other) const
{
	return __type_name < other.__type_name;
}
const char* type_info::name() const
{
	return __type_name;
}
type_info::type_info (const type_info& rhs)
{
	__type_name = rhs.__type_name;
}
type_info& type_info::operator= (const type_info& rhs) 
{
	return *new type_info(rhs);
}

ABI_NAMESPACE::__fundamental_type_info::~__fundamental_type_info() {}
ABI_NAMESPACE::__array_type_info::~__array_type_info() {}
ABI_NAMESPACE::__function_type_info::~__function_type_info() {}
ABI_NAMESPACE::__enum_type_info::~__enum_type_info() {}
ABI_NAMESPACE::__class_type_info::~__class_type_info() {}
ABI_NAMESPACE::__si_class_type_info::~__si_class_type_info() {}
ABI_NAMESPACE::__vmi_class_type_info::~__vmi_class_type_info() {}
ABI_NAMESPACE::__pbase_type_info::~__pbase_type_info() {}
ABI_NAMESPACE::__pointer_type_info::~__pointer_type_info() {}
ABI_NAMESPACE::__pointer_to_member_type_info::~__pointer_to_member_type_info() {}

// From libelftc
extern "C" char    *__cxa_demangle_gnu3(const char *);

extern "C" char* __cxa_demangle(const char* mangled_name,
                                char* buf,
                                size_t* n,
                                int* status)
{
	// TODO: We should probably just be linking against libelf-tc, rather than
	// copying their code.  This requires them to do an actual release,
	// however, and for our changes to be pushed upstream.  We also need to
	// call a different demangling function here depending on the ABI (e.g.
	// ARM).
	char *demangled = __cxa_demangle_gnu3(mangled_name);
	if (NULL != demangled)
	{
		size_t len = strlen(demangled);
		buf = (char*)realloc(buf, len+1);
		if (0 != buf)
		{
			memcpy(buf, demangled, len);
			buf[len] = 0;
			if (n)
			{
				*n = len;
			}
			if (status)
			{
				*status = 0;
			}
		}
		else
		{
			if (status)
			{
				*status = -1;
			}
		}
		free(demangled);
	}
	else
	{
		if (status)
		{
			*status = -2;
		}
		return NULL;
	}
	return buf;
}