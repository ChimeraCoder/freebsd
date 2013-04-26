
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

#include <string.h>
#include <openssl/des.h>
#include <openssl/ui.h>
#include <openssl/crypto.h>

int DES_read_password(DES_cblock *key, const char *prompt, int verify)
	{
	int ok;
	char buf[BUFSIZ],buff[BUFSIZ];

	if ((ok=UI_UTIL_read_pw(buf,buff,BUFSIZ,prompt,verify)) == 0)
		DES_string_to_key(buf,key);
	OPENSSL_cleanse(buf,BUFSIZ);
	OPENSSL_cleanse(buff,BUFSIZ);
	return(ok);
	}

int DES_read_2passwords(DES_cblock *key1, DES_cblock *key2, const char *prompt,
	     int verify)
	{
	int ok;
	char buf[BUFSIZ],buff[BUFSIZ];

	if ((ok=UI_UTIL_read_pw(buf,buff,BUFSIZ,prompt,verify)) == 0)
		DES_string_to_2keys(buf,key1,key2);
	OPENSSL_cleanse(buf,BUFSIZ);
	OPENSSL_cleanse(buff,BUFSIZ);
	return(ok);
	}