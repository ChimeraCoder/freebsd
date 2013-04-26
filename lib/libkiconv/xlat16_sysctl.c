
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

/*
 * kiconv(3) requires shared linked, and reduce module size
 * when statically linked.
 */

#ifdef PIC

#include <sys/types.h>
#include <sys/iconv.h>
#include <sys/sysctl.h>

#include <ctype.h>
#include <errno.h>
#include <string.h>

int
kiconv_add_xlat16_table(const char *to, const char *from, const void *data, int datalen)
{
	struct iconv_add_in din;
	struct iconv_add_out dout;
	size_t olen;

	if (strlen(from) >= ICONV_CSNMAXLEN || strlen(to) >= ICONV_CSNMAXLEN)
		return (EINVAL);
	din.ia_version = ICONV_ADD_VER;
	strcpy(din.ia_converter, "xlat16");
	strcpy(din.ia_from, from);
	strcpy(din.ia_to, to);
	din.ia_data = data;
	din.ia_datalen = datalen;
	olen = sizeof(dout);
	if (sysctlbyname("kern.iconv.add", &dout, &olen, &din, sizeof(din)) == -1)
		return (errno);
	return (0);
}

#else /* statically linked */

#include <sys/types.h>
#include <sys/iconv.h>
#include <errno.h>

int
kiconv_add_xlat16_table(const char *to __unused, const char *from __unused,
    const void *data __unused, int datalen __unused)
{

	return (EINVAL);
}

#endif /* PIC */