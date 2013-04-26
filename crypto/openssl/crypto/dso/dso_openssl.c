
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

#include <stdio.h>
#include "cryptlib.h"
#include <openssl/dso.h>

/* We just pinch the method from an appropriate "default" method. */

DSO_METHOD *DSO_METHOD_openssl(void)
	{
#ifdef DEF_DSO_METHOD
	return(DEF_DSO_METHOD());
#elif defined(DSO_DLFCN)
	return(DSO_METHOD_dlfcn());
#elif defined(DSO_DL)
	return(DSO_METHOD_dl());
#elif defined(DSO_WIN32)
	return(DSO_METHOD_win32());
#elif defined(DSO_VMS)
	return(DSO_METHOD_vms());
#elif defined(DSO_BEOS)
	return(DSO_METHOD_beos());
#else
	return(DSO_METHOD_null());
#endif
	}