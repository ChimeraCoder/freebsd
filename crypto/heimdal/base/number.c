
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

static void
number_dealloc(void *ptr)
{
}

static int
number_cmp(void *a, void *b)
{
    int na, nb;

    if (heim_base_is_tagged_object(a))
	na = heim_base_tagged_object_value(a);
    else
	na = *(int *)a;

    if (heim_base_is_tagged_object(b))
	nb = heim_base_tagged_object_value(b);
    else
	nb = *(int *)b;

    return na - nb;
}

static unsigned long
number_hash(void *ptr)
{
    if (heim_base_is_tagged_object(ptr))
	return heim_base_tagged_object_value(ptr);
    return (unsigned long)*(int *)ptr;
}

struct heim_type_data _heim_number_object = {
    HEIM_TID_NUMBER,
    "number-object",
    NULL,
    number_dealloc,
    NULL,
    number_cmp,
    number_hash
};

/**
 * Create a number object
 *
 * @param the number to contain in the object
 *
 * @return a number object
 */

heim_number_t
heim_number_create(int number)
{
    heim_number_t n;

    if (number < 0xffffff && number >= 0)
	return heim_base_make_tagged_object(number, HEIM_TID_NUMBER);

    n = _heim_alloc_object(&_heim_number_object, sizeof(int));
    if (n)
	*((int *)n) = number;
    return n;
}

/**
 * Return the type ID of number objects
 *
 * @return type id of number objects
 */

heim_tid_t
heim_number_get_type_id(void)
{
    return HEIM_TID_NUMBER;
}

/**
 * Get the int value of the content
 *
 * @param number the number object to get the value from
 *
 * @return an int
 */

int
heim_number_get_int(heim_number_t number)
{
    if (heim_base_is_tagged_object(number))
	return heim_base_tagged_object_value(number);
    return *(int *)number;
}