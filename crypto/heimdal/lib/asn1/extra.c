
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
#include "heim_asn1.h"

RCSID("$Id$");

int
encode_heim_any(unsigned char *p, size_t len,
		const heim_any *data, size_t *size)
{
    return der_put_octet_string (p, len, data, size);
}

int
decode_heim_any(const unsigned char *p, size_t len,
		heim_any *data, size_t *size)
{
    size_t len_len, length, l;
    Der_class thisclass;
    Der_type thistype;
    unsigned int thistag;
    int e;

    memset(data, 0, sizeof(*data));

    e = der_get_tag (p, len, &thisclass, &thistype, &thistag, &l);
    if (e) return e;
    if (l > len)
	return ASN1_OVERFLOW;
    e = der_get_length(p + l, len - l, &length, &len_len);
    if (e) return e;
    if (length == ASN1_INDEFINITE) {
        if (len < len_len + l)
	    return ASN1_OVERFLOW;
	length = len - (len_len + l);
    } else {
	if (len < length + len_len + l)
	    return ASN1_OVERFLOW;
    }

    data->data = malloc(length + len_len + l);
    if (data->data == NULL)
	return ENOMEM;
    data->length = length + len_len + l;
    memcpy(data->data, p, length + len_len + l);

    if (size)
	*size = length + len_len + l;

    return 0;
}

void
free_heim_any(heim_any *data)
{
    der_free_octet_string(data);
}

size_t
length_heim_any(const heim_any *data)
{
    return data->length;
}

int
copy_heim_any(const heim_any *from, heim_any *to)
{
    return der_copy_octet_string(from, to);
}

int
encode_heim_any_set(unsigned char *p, size_t len,
		    const heim_any_set *data, size_t *size)
{
    return der_put_octet_string (p, len, data, size);
}

int
decode_heim_any_set(const unsigned char *p, size_t len,
		heim_any_set *data, size_t *size)
{
    return der_get_octet_string(p, len, data, size);
}

void
free_heim_any_set(heim_any_set *data)
{
    der_free_octet_string(data);
}

size_t
length_heim_any_set(const heim_any *data)
{
    return data->length;
}

int
copy_heim_any_set(const heim_any_set *from, heim_any_set *to)
{
    return der_copy_octet_string(from, to);
}

int
heim_any_cmp(const heim_any_set *p, const heim_any_set *q)
{
    return der_heim_octet_string_cmp(p, q);
}