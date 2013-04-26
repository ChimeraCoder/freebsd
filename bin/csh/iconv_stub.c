
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

#include <dlfcn.h>
#include <stddef.h>

#include "iconv.h"

#undef iconv_open
#undef iconv
#undef iconv_close

#define ICONVLIB	"libiconv.so"
#define ICONV_ENGINE	"iconv"
#define ICONV_OPEN	"iconv_open"
#define ICONV_CLOSE	"iconv_close"

typedef iconv_t iconv_open_t(const char *, const char *);

dl_iconv_t *dl_iconv;
dl_iconv_close_t *dl_iconv_close;

static int initialized;
static void *iconvlib;
static iconv_open_t *iconv_open;

iconv_t
dl_iconv_open(const char *tocode, const char *fromcode)
{
	if (initialized) {
		if (iconvlib == NULL)
			return (iconv_t)-1;
	} else {
		initialized = 1;
		iconvlib = dlopen(ICONVLIB, RTLD_LAZY | RTLD_GLOBAL);
		if (iconvlib == NULL)
			return (iconv_t)-1;
		iconv_open = (iconv_open_t *)dlfunc(iconvlib, ICONV_OPEN);
		if (iconv_open == NULL)
			goto dlfunc_err;
		dl_iconv = (dl_iconv_t *)dlfunc(iconvlib, ICONV_ENGINE);
		if (dl_iconv == NULL)
			goto dlfunc_err;
		dl_iconv_close = (dl_iconv_close_t *)dlfunc(iconvlib,
		    ICONV_CLOSE);
		if (dl_iconv_close == NULL)
			goto dlfunc_err;
	}
	return iconv_open(tocode, fromcode);

dlfunc_err:
	dlclose(iconvlib);
	iconvlib = NULL;
	return (iconv_t)-1;
}