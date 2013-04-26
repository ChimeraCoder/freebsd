
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

void
der_free_general_string (heim_general_string *str)
{
    free(*str);
    *str = NULL;
}

void
der_free_integer (int *i)
{
    *i = 0;
}

void
der_free_unsigned (unsigned *u)
{
    *u = 0;
}

void
der_free_generalized_time(time_t *t)
{
    *t = 0;
}

void
der_free_utctime(time_t *t)
{
    *t = 0;
}


void
der_free_utf8string (heim_utf8_string *str)
{
    free(*str);
    *str = NULL;
}

void
der_free_printable_string (heim_printable_string *str)
{
    der_free_octet_string(str);
}

void
der_free_ia5_string (heim_ia5_string *str)
{
    der_free_octet_string(str);
}

void
der_free_bmp_string (heim_bmp_string *k)
{
    free(k->data);
    k->data = NULL;
    k->length = 0;
}

void
der_free_universal_string (heim_universal_string *k)
{
    free(k->data);
    k->data = NULL;
    k->length = 0;
}

void
der_free_visible_string (heim_visible_string *str)
{
    free(*str);
    *str = NULL;
}

void
der_free_octet_string (heim_octet_string *k)
{
    free(k->data);
    k->data = NULL;
    k->length = 0;
}

void
der_free_heim_integer (heim_integer *k)
{
    free(k->data);
    k->data = NULL;
    k->length = 0;
}

void
der_free_oid (heim_oid *k)
{
    free(k->components);
    k->components = NULL;
    k->length = 0;
}

void
der_free_bit_string (heim_bit_string *k)
{
    free(k->data);
    k->data = NULL;
    k->length = 0;
}