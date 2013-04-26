
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

#include "cryptlib.h"

#ifndef NO_WINDOWS_BRAINDEATH
#include "buildinf.h"
#endif

const char *SSLeay_version(int t)
	{
	if (t == SSLEAY_VERSION)
		return OPENSSL_VERSION_TEXT;
	if (t == SSLEAY_BUILT_ON)
		{
#ifdef DATE
		static char buf[sizeof(DATE)+11];

		BIO_snprintf(buf,sizeof buf,"built on: %s",DATE);
		return(buf);
#else
		return("built on: date not available");
#endif
		}
	if (t == SSLEAY_CFLAGS)
		{
#ifdef CFLAGS
		static char buf[sizeof(CFLAGS)+11];

		BIO_snprintf(buf,sizeof buf,"compiler: %s",CFLAGS);
		return(buf);
#else
		return("compiler: information not available");
#endif
		}
	if (t == SSLEAY_PLATFORM)
		{
#ifdef PLATFORM
		static char buf[sizeof(PLATFORM)+11];

		BIO_snprintf(buf,sizeof buf,"platform: %s", PLATFORM);
		return(buf);
#else
		return("platform: information not available");
#endif
		}
	if (t == SSLEAY_DIR)
		{
#ifdef OPENSSLDIR
		return "OPENSSLDIR: \"" OPENSSLDIR "\"";
#else
		return "OPENSSLDIR: N/A";
#endif
		}
	return("not available");
	}

unsigned long SSLeay(void)
	{
	return(SSLEAY_VERSION_NUMBER);
	}