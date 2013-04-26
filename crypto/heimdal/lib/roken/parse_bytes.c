
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

#include <config.h>

#include <parse_units.h>
#include "parse_bytes.h"

static struct units bytes_units[] = {
    { "gigabyte", 1024 * 1024 * 1024 },
    { "gbyte", 1024 * 1024 * 1024 },
    { "GB", 1024 * 1024 * 1024 },
    { "megabyte", 1024 * 1024 },
    { "mbyte", 1024 * 1024 },
    { "MB", 1024 * 1024 },
    { "kilobyte", 1024 },
    { "KB", 1024 },
    { "byte", 1 },
    { NULL, 0 }
};

static struct units bytes_short_units[] = {
    { "GB", 1024 * 1024 * 1024 },
    { "MB", 1024 * 1024 },
    { "KB", 1024 },
    { NULL, 0 }
};

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
parse_bytes (const char *s, const char *def_unit)
{
    return parse_units (s, bytes_units, def_unit);
}

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
unparse_bytes (int t, char *s, size_t len)
{
    return unparse_units (t, bytes_units, s, len);
}

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
unparse_bytes_short (int t, char *s, size_t len)
{
    return unparse_units_approx (t, bytes_short_units, s, len);
}