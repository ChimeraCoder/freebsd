
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

#include <sys/types.h>
#include <sys/iconv.h>
#include <sys/sysctl.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#ifdef APPLE
#include <sys/types.h>
extern uid_t real_uid, eff_uid;
#endif

int
kiconv_add_xlat_table(const char *to, const char *from, const u_char *table)
{
	struct iconv_add_in din;
	struct iconv_add_out dout;
	size_t olen;

	if (strlen(from) >= ICONV_CSNMAXLEN || strlen(to) >= ICONV_CSNMAXLEN)
		return EINVAL;
	din.ia_version = ICONV_ADD_VER;
	strcpy(din.ia_converter, "xlat");
	strcpy(din.ia_from, from);
	strcpy(din.ia_to, to);
	din.ia_data = table;
	din.ia_datalen = 256;
	olen = sizeof(dout);
#ifdef APPLE
        seteuid(eff_uid); /* restore setuid root briefly */
	if (sysctlbyname("net.smb.fs.iconv.add", &dout, &olen, &din, sizeof(din)) == -1) {
        	seteuid(real_uid); /* and back to real user */
		return errno;
	}
        seteuid(real_uid); /* and back to real user */
#else
	if (sysctlbyname("kern.iconv.add", &dout, &olen, &din, sizeof(din)) == -1)
		return errno;
#endif
	return 0;
}