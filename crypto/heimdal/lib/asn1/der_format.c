
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
#include <hex.h>

RCSID("$Id$");

int
der_parse_hex_heim_integer (const char *p, heim_integer *data)
{
    ssize_t len;

    data->length = 0;
    data->negative = 0;
    data->data = NULL;

    if (*p == '-') {
	p++;
	data->negative = 1;
    }

    len = strlen(p);
    if (len <= 0) {
	data->data = NULL;
	data->length = 0;
	return EINVAL;
    }

    data->length = (len / 2) + 1;
    data->data = malloc(data->length);
    if (data->data == NULL) {
	data->length = 0;
	return ENOMEM;
    }

    len = hex_decode(p, data->data, data->length);
    if (len < 0) {
	free(data->data);
	data->data = NULL;
	data->length = 0;
	return EINVAL;
    }

    {
	unsigned char *q = data->data;
	while(len > 0 && *q == 0) {
	    q++;
	    len--;
	}
	data->length = len;
	memmove(data->data, q, len);
    }
    return 0;
}

int
der_print_hex_heim_integer (const heim_integer *data, char **p)
{
    ssize_t len;
    char *q;

    len = hex_encode(data->data, data->length, p);
    if (len < 0)
	return ENOMEM;

    if (data->negative) {
	len = asprintf(&q, "-%s", *p);
	free(*p);
	if (len < 0)
	    return ENOMEM;
	*p = q;
    }
    return 0;
}

int
der_print_heim_oid (const heim_oid *oid, char delim, char **str)
{
    struct rk_strpool *p = NULL;
    size_t i;

    if (oid->length == 0)
	return EINVAL;

    for (i = 0; i < oid->length ; i++) {
	p = rk_strpoolprintf(p, "%d", oid->components[i]);
	if (p && i < oid->length - 1)
	    p = rk_strpoolprintf(p, "%c", delim);
	if (p == NULL) {
	    *str = NULL;
	    return ENOMEM;
	}
    }

    *str = rk_strpoolcollect(p);
    if (*str == NULL)
	return ENOMEM;
    return 0;
}

int
der_parse_heim_oid (const char *str, const char *sep, heim_oid *data)
{
    char *s, *w, *brkt, *endptr;
    unsigned int *c;
    long l;

    data->length = 0;
    data->components = NULL;

    if (sep == NULL)
	sep = ".";

    s = strdup(str);

    for (w = strtok_r(s, sep, &brkt);
	 w != NULL;
	 w = strtok_r(NULL, sep, &brkt)) {

	c = realloc(data->components,
		    (data->length + 1) * sizeof(data->components[0]));
	if (c == NULL) {
	    der_free_oid(data);
	    free(s);
	    return ENOMEM;
	}
	data->components = c;

	l = strtol(w, &endptr, 10);
	if (*endptr != '\0' || l < 0 || l > INT_MAX) {
	    der_free_oid(data);
	    free(s);
	    return EINVAL;
	}
	data->components[data->length++] = l;
    }
    free(s);
    return 0;
}