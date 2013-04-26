
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

/* $Id$ */

#include <string.h>
#include <stdlib.h>
#include <krb5.h>

const char* check_length(krb5_context, krb5_principal, krb5_data *);

/* specify the api-version this library conforms to */

int version = 0;

/* just check the length of the password, this is what the default
   check does, but this lets you specify the minimum length in
   krb5.conf */
const char*
check_length(krb5_context context,
             krb5_principal prinipal,
             krb5_data *password)
{
    int min_length = krb5_config_get_int_default(context, NULL, 6,
						 "password_quality",
						 "min_length",
						 NULL);
    if(password->length < min_length)
	return "Password too short";
    return NULL;
}

#ifdef DICTPATH

/* use cracklib to check password quality; this requires a patch for
   cracklib that can be found at
   ftp://ftp.pdc.kth.se/pub/krb/src/cracklib.patch */

const char*
check_cracklib(krb5_context context,
	       krb5_principal principal,
	       krb5_data *password)
{
    char *s = malloc(password->length + 1);
    char *msg;
    char *strings[2];
    if(s == NULL)
	return NULL; /* XXX */
    strings[0] = principal->name.name_string.val[0]; /* XXX */
    strings[1] = NULL;
    memcpy(s, password->data, password->length);
    s[password->length] = '\0';
    msg = FascistCheck(s, DICTPATH, strings);
    memset(s, 0, password->length);
    free(s);
    return msg;
}
#endif