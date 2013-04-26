
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

#include <sm/gen.h>
SM_RCSID("@(#)$Id: snprintf.c,v 1.24 2006/10/12 21:50:10 ca Exp $")
#include <limits.h>
#include <sm/varargs.h>
#include <sm/io.h>
#include <sm/string.h>
#include "local.h"

/*
**  SM_SNPRINTF -- format a string to a memory location of restricted size
**
**	Parameters:
**		str -- memory location to place formatted string
**		n -- size of buffer pointed to by str, capped to
**			a maximum of INT_MAX
**		fmt -- the formatting directives
**		... -- the data to satisfy the formatting
**
**	Returns:
**		Failure: -1
**		Success: number of bytes that would have been written
**			to str, not including the trailing '\0',
**			up to a maximum of INT_MAX, as if there was
**			no buffer size limitation.  If the result >= n
**			then the output was truncated.
**
**	Side Effects:
**		If n > 0, then between 0 and n-1 bytes of formatted output
**		are written into 'str', followed by a '\0'.
*/

int
#if SM_VA_STD
sm_snprintf(char *str, size_t n, char const *fmt, ...)
#else /* SM_VA_STD */
sm_snprintf(str, n, fmt, va_alist)
	char *str;
	size_t n;
	char *fmt;
	va_dcl
#endif /* SM_VA_STD */
{
	int ret;
	SM_VA_LOCAL_DECL
	SM_FILE_T fake;

	/* While snprintf(3) specifies size_t stdio uses an int internally */
	if (n > INT_MAX)
		n = INT_MAX;
	SM_VA_START(ap, fmt);

	/* XXX put this into a static? */
	fake.sm_magic = SmFileMagic;
	fake.f_file = -1;
	fake.f_flags = SMWR | SMSTR;
	fake.f_cookie = &fake;
	fake.f_bf.smb_base = fake.f_p = (unsigned char *)str;
	fake.f_bf.smb_size = fake.f_w = n ? n - 1 : 0;
	fake.f_timeout = SM_TIME_FOREVER;
	fake.f_timeoutstate = SM_TIME_BLOCK;
	fake.f_close = NULL;
	fake.f_open = NULL;
	fake.f_read = NULL;
	fake.f_write = NULL;
	fake.f_seek = NULL;
	fake.f_setinfo = fake.f_getinfo = NULL;
	fake.f_type = "sm_snprintf:fake";
	ret = sm_io_vfprintf(&fake, SM_TIME_FOREVER, fmt, ap);
	if (n > 0)
		*fake.f_p = '\0';
	SM_VA_END(ap);
	return ret;
}