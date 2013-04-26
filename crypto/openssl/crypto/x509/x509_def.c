
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
#include <openssl/crypto.h>
#include <openssl/x509.h>

const char *X509_get_default_private_dir(void)
	{ return(X509_PRIVATE_DIR); }
	
const char *X509_get_default_cert_area(void)
	{ return(X509_CERT_AREA); }

const char *X509_get_default_cert_dir(void)
	{ return(X509_CERT_DIR); }

const char *X509_get_default_cert_file(void)
	{ return(X509_CERT_FILE); }

const char *X509_get_default_cert_dir_env(void)
	{ return(X509_CERT_DIR_EVP); }

const char *X509_get_default_cert_file_env(void)
	{ return(X509_CERT_FILE_EVP); }