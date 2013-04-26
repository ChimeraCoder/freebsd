
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

#include <openssl/crypto.h>
#include <openssl/opensslconf.h>

#if defined(__OpenBSD__) || (defined(__FreeBSD__) && __FreeBSD__ > 2)

#include OPENSSL_UNISTD

int OPENSSL_issetugid(void)
	{
	return issetugid();
	}

#elif defined(OPENSSL_SYS_WIN32) || defined(OPENSSL_SYS_VXWORKS) || defined(OPENSSL_SYS_NETWARE)

int OPENSSL_issetugid(void)
	{
	return 0;
	}

#else

#include OPENSSL_UNISTD
#include <sys/types.h>

int OPENSSL_issetugid(void)
	{
	if (getuid() != geteuid()) return 1;
	if (getgid() != getegid()) return 1;
	return 0;
	}
#endif