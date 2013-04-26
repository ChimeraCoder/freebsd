
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

int
der_heim_oid_cmp(const heim_oid *p, const heim_oid *q)
{
    if (p->length != q->length)
	return p->length - q->length;
    return memcmp(p->components,
		  q->components,
		  p->length * sizeof(*p->components));
}

int
der_heim_octet_string_cmp(const heim_octet_string *p,
			  const heim_octet_string *q)
{
    if (p->length != q->length)
	return p->length - q->length;
    return memcmp(p->data, q->data, p->length);
}

int
der_printable_string_cmp(const heim_printable_string *p,
			 const heim_printable_string *q)
{
    return der_heim_octet_string_cmp(p, q);
}

int
der_ia5_string_cmp(const heim_ia5_string *p,
		   const heim_ia5_string *q)
{
    return der_heim_octet_string_cmp(p, q);
}

int
der_heim_bit_string_cmp(const heim_bit_string *p,
			const heim_bit_string *q)
{
    int i, r1, r2;
    if (p->length != q->length)
	return p->length - q->length;
    i = memcmp(p->data, q->data, p->length / 8);
    if (i)
	return i;
    if ((p->length % 8) == 0)
	return 0;
    i = (p->length / 8);
    r1 = ((unsigned char *)p->data)[i];
    r2 = ((unsigned char *)q->data)[i];
    i = 8 - (p->length % 8);
    r1 = r1 >> i;
    r2 = r2 >> i;
    return r1 - r2;
}

int
der_heim_integer_cmp(const heim_integer *p,
		     const heim_integer *q)
{
    if (p->negative != q->negative)
	return q->negative - p->negative;
    if (p->length != q->length)
	return p->length - q->length;
    return memcmp(p->data, q->data, p->length);
}

int
der_heim_bmp_string_cmp(const heim_bmp_string *p, const heim_bmp_string *q)
{
    if (p->length != q->length)
	return p->length - q->length;
    return memcmp(p->data, q->data, q->length * sizeof(q->data[0]));
}

int
der_heim_universal_string_cmp(const heim_universal_string *p,
			      const heim_universal_string *q)
{
    if (p->length != q->length)
	return p->length - q->length;
    return memcmp(p->data, q->data, q->length * sizeof(q->data[0]));
}