
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

struct heim_type_data _heim_null_object = {
    HEIM_TID_NULL,
    "null-object",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

heim_null_t
heim_null_create(void)
{
    return heim_base_make_tagged_object(0, HEIM_TID_NULL);
}