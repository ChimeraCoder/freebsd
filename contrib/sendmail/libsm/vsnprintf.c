
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
SM_RCSID("@(#)$Id: vsnprintf.c,v 1.23 2001/09/11 04:04:49 gshapiro Exp $")
#include <limits.h>
#include <sm/io.h>
#include "local.h"

/*
**  SM_VSNPRINTF -- format data for "output" into a string
**
**	Assigned 'str' to a "fake" file pointer. This allows common
**	o/p formatting function sm_vprintf() to be used.
**
**	Parameters:
**		str -- location for output
**		n -- maximum size for o/p
**		fmt -- format directives
**		ap -- data unit vectors for use by 'fmt'
**
**	Results:
**		result from sm_io_vfprintf()
**
**	Side Effects:
**		Limits the size ('n') to INT_MAX.
*/

int
sm_vsnprintf(str, n, fmt, ap)
	char *str;
	size_t n;
	const char *fmt;
	SM_VA_LOCAL_DECL
{
	int ret;
	char dummy;
	SM_FILE_T fake;

	/* While snprintf(3) specifies size_t stdio uses an int internally */
	if (n > INT_MAX)
		n = INT_MAX;

	/* Stdio internals do not deal correctly with zero length buffer */
	if (n == 0)
	{
		str = &dummy;
		n = 1;
	}
	fake.sm_magic = SmFileMagic;
	fake.f_timeout = SM_TIME_FOREVER;
	fake.f_timeoutstate = SM_TIME_BLOCK;
	fake.f_file = -1;
	fake.f_flags = SMWR | SMSTR;
	fake.f_bf.smb_base = fake.f_p = (unsigned char *)str;
	fake.f_bf.smb_size = fake.f_w = n - 1;
	fake.f_close = NULL;
	fake.f_open = NULL;
	fake.f_read = NULL;
	fake.f_write = NULL;
	fake.f_seek = NULL;
	fake.f_setinfo = fake.f_getinfo = NULL;
	fake.f_type = "sm_vsnprintf:fake";
	ret = sm_io_vfprintf(&fake, SM_TIME_FOREVER, fmt, ap);
	*fake.f_p = 0;
	return ret;
}