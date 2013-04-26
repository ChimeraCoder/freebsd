
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

struct heim_type_data _heim_bool_object = {
    HEIM_TID_BOOL,
    "bool-object",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

heim_bool_t
heim_bool_create(int val)
{
    return heim_base_make_tagged_object(!!val, HEIM_TID_BOOL);
}

int
heim_bool_val(heim_bool_t ptr)
{
    return heim_base_tagged_object_value(ptr);
}