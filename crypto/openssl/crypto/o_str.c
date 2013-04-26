
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

#include <ctype.h>
#include <e_os.h>
#include "o_str.h"

#if !defined(OPENSSL_IMPLEMENTS_strncasecmp) && \
    !defined(OPENSSL_SYSNAME_WIN32) && \
    !defined(NETWARE_CLIB)
# include <strings.h>
#endif

int OPENSSL_strncasecmp(const char *str1, const char *str2, size_t n)
	{
#if defined(OPENSSL_IMPLEMENTS_strncasecmp)
	while (*str1 && *str2 && n)
		{
		int res = toupper(*str1) - toupper(*str2);
		if (res) return res < 0 ? -1 : 1;
		str1++;
		str2++;
		n--;
		}
	if (n == 0)
		return 0;
	if (*str1)
		return 1;
	if (*str2)
		return -1;
	return 0;
#else
	/* Recursion hazard warning! Whenever strncasecmp is #defined as
	 * OPENSSL_strncasecmp, OPENSSL_IMPLEMENTS_strncasecmp must be
	 * defined as well. */
	return strncasecmp(str1, str2, n);
#endif
	}
int OPENSSL_strcasecmp(const char *str1, const char *str2)
	{
#if defined(OPENSSL_IMPLEMENTS_strncasecmp)
	return OPENSSL_strncasecmp(str1, str2, (size_t)-1);
#else
	return strcasecmp(str1, str2);
#endif
	}

int OPENSSL_memcmp(const void *v1,const void *v2,size_t n)
	{
	const unsigned char *c1=v1,*c2=v2;
	int ret=0;

	while(n && (ret=*c1-*c2)==0) n--,c1++,c2++;

	return ret;
	}