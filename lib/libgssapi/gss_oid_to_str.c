
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

#include <gssapi/gssapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "utils.h"

OM_uint32
gss_oid_to_str(OM_uint32 *minor_status, gss_OID oid, gss_buffer_t oid_str)
{
	char		numstr[128];
	unsigned long	number;
	int		numshift;
	size_t		string_length;
	size_t		i;
	unsigned char	*cp;
	char		*bp;

	*minor_status = 0;
	_gss_buffer_zero(oid_str);

	if (oid == GSS_C_NULL_OID)
		return (GSS_S_FAILURE);

	/* Decoded according to krb5/gssapi_krb5.c */

	/* First determine the size of the string */
	string_length = 0;
	number = 0;
	numshift = 0;
	cp = (unsigned char *) oid->elements;
	number = (unsigned long) cp[0];
	sprintf(numstr, "%ld ", number/40);
	string_length += strlen(numstr);
	sprintf(numstr, "%ld ", number%40);
	string_length += strlen(numstr);
	for (i=1; i<oid->length; i++) {
		if ( (size_t) (numshift+7) < (sizeof(unsigned long)*8)) {
			number = (number << 7) | (cp[i] & 0x7f);
			numshift += 7;
		}
		else {
			*minor_status = 0;
			return(GSS_S_FAILURE);
		}
		if ((cp[i] & 0x80) == 0) {
			sprintf(numstr, "%ld ", number);
			string_length += strlen(numstr);
			number = 0;
			numshift = 0;
		}
	}
	/*
	 * If we get here, we've calculated the length of "n n n ... n ".
	 * Add 4 here for "{ " and "}\0".
	 */
	string_length += 4;
	if ((bp = (char *) malloc(string_length))) {
		strcpy(bp, "{ ");
		number = (unsigned long) cp[0];
		sprintf(numstr, "%ld ", number/40);
		strcat(bp, numstr);
		sprintf(numstr, "%ld ", number%40);
		strcat(bp, numstr);
		number = 0;
		cp = (unsigned char *) oid->elements;
		for (i=1; i<oid->length; i++) {
			number = (number << 7) | (cp[i] & 0x7f);
			if ((cp[i] & 0x80) == 0) {
				sprintf(numstr, "%ld ", number);
				strcat(bp, numstr);
				number = 0;
			}
		}
		strcat(bp, "}");
		oid_str->length = strlen(bp)+1;
		oid_str->value = (void *) bp;
		*minor_status = 0;
		return(GSS_S_COMPLETE);
	}
	*minor_status = ENOMEM;
	return(GSS_S_FAILURE);
}