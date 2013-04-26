
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

#include "baselocl.h"
#include <string.h>

static void
string_dealloc(void *ptr)
{
}

static int
string_cmp(void *a, void *b)
{
    return strcmp(a, b);
}

static unsigned long
string_hash(void *ptr)
{
    const char *s = ptr;
    unsigned long n;

    for (n = 0; *s; ++s)
	n += *s;
    return n;
}

struct heim_type_data _heim_string_object = {
    HEIM_TID_STRING,
    "string-object",
    NULL,
    string_dealloc,
    NULL,
    string_cmp,
    string_hash
};

/**
 * Create a string object
 *
 * @param string the string to create, must be an utf8 string
 *
 * @return string object
 */

heim_string_t
heim_string_create(const char *string)
{
    size_t len = strlen(string);
    heim_string_t s;

    s = _heim_alloc_object(&_heim_string_object, len + 1);
    if (s)
	memcpy(s, string, len + 1);
    return s;
}

/**
 * Return the type ID of string objects
 *
 * @return type id of string objects
 */

heim_tid_t
heim_string_get_type_id(void)
{
    return HEIM_TID_STRING;
}

/**
 * Get the string value of the content.
 *
 * @param string the string object to get the value from
 *
 * @return a utf8 string
 */

const char *
heim_string_get_utf8(heim_string_t string)
{
    return (const char *)string;
}