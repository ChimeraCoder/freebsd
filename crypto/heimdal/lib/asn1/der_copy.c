
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

#include "der_locl.h"

RCSID("$Id$");

int
der_copy_general_string (const heim_general_string *from,
			 heim_general_string *to)
{
    *to = strdup(*from);
    if(*to == NULL)
	return ENOMEM;
    return 0;
}

int
der_copy_integer (const int *from, int *to)
{
    *to = *from;
    return 0;
}

int
der_copy_unsigned (const unsigned *from, unsigned *to)
{
    *to = *from;
    return 0;
}

int
der_copy_generalized_time (const time_t *from, time_t *to)
{
    *to = *from;
    return 0;
}

int
der_copy_utctime (const time_t *from, time_t *to)
{
    *to = *from;
    return 0;
}

int
der_copy_utf8string (const heim_utf8_string *from, heim_utf8_string *to)
{
    return der_copy_general_string(from, to);
}

int
der_copy_printable_string (const heim_printable_string *from,
		       heim_printable_string *to)
{
    to->length = from->length;
    to->data   = malloc(to->length + 1);
    if(to->data == NULL)
	return ENOMEM;
    memcpy(to->data, from->data, to->length);
    ((char *)to->data)[to->length] = '\0';
    return 0;
}

int
der_copy_ia5_string (const heim_ia5_string *from,
		     heim_ia5_string *to)
{
    return der_copy_printable_string(from, to);
}

int
der_copy_bmp_string (const heim_bmp_string *from, heim_bmp_string *to)
{
    to->length = from->length;
    to->data   = malloc(to->length * sizeof(to->data[0]));
    if(to->length != 0 && to->data == NULL)
	return ENOMEM;
    memcpy(to->data, from->data, to->length * sizeof(to->data[0]));
    return 0;
}

int
der_copy_universal_string (const heim_universal_string *from,
			   heim_universal_string *to)
{
    to->length = from->length;
    to->data   = malloc(to->length * sizeof(to->data[0]));
    if(to->length != 0 && to->data == NULL)
	return ENOMEM;
    memcpy(to->data, from->data, to->length * sizeof(to->data[0]));
    return 0;
}

int
der_copy_visible_string (const heim_visible_string *from,
			 heim_visible_string *to)
{
    return der_copy_general_string(from, to);
}

int
der_copy_octet_string (const heim_octet_string *from, heim_octet_string *to)
{
    to->length = from->length;
    to->data   = malloc(to->length);
    if(to->length != 0 && to->data == NULL)
	return ENOMEM;
    memcpy(to->data, from->data, to->length);
    return 0;
}

int
der_copy_heim_integer (const heim_integer *from, heim_integer *to)
{
    to->length = from->length;
    to->data   = malloc(to->length);
    if(to->length != 0 && to->data == NULL)
	return ENOMEM;
    memcpy(to->data, from->data, to->length);
    to->negative = from->negative;
    return 0;
}

int
der_copy_oid (const heim_oid *from, heim_oid *to)
{
    to->length     = from->length;
    to->components = malloc(to->length * sizeof(*to->components));
    if (to->length != 0 && to->components == NULL)
	return ENOMEM;
    memcpy(to->components, from->components,
	   to->length * sizeof(*to->components));
    return 0;
}

int
der_copy_bit_string (const heim_bit_string *from, heim_bit_string *to)
{
    size_t len;

    len = (from->length + 7) / 8;
    to->length = from->length;
    to->data   = malloc(len);
    if(len != 0 && to->data == NULL)
	return ENOMEM;
    memcpy(to->data, from->data, len);
    return 0;
}