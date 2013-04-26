
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

#include "krb5_locl.h"

#ifndef HEIMDAL_SMALLER
#define DES3_OLD_ENCTYPE 1
#endif

static struct _krb5_key_type keytype_null = {
    ENCTYPE_NULL,
    "null",
    0,
    0,
    0,
    NULL,
    NULL,
    NULL
};

static krb5_error_code
NONE_checksum(krb5_context context,
	      struct _krb5_key_data *key,
	      const void *data,
	      size_t len,
	      unsigned usage,
	      Checksum *C)
{
    return 0;
}

struct _krb5_checksum_type _krb5_checksum_none = {
    CKSUMTYPE_NONE,
    "none",
    1,
    0,
    0,
    NONE_checksum,
    NULL
};

static krb5_error_code
NULL_encrypt(krb5_context context,
	     struct _krb5_key_data *key,
	     void *data,
	     size_t len,
	     krb5_boolean encryptp,
	     int usage,
	     void *ivec)
{
    return 0;
}

struct _krb5_encryption_type _krb5_enctype_null = {
    ETYPE_NULL,
    "null",
    1,
    1,
    0,
    &keytype_null,
    &_krb5_checksum_none,
    NULL,
    F_DISABLED,
    NULL_encrypt,
    0,
    NULL
};