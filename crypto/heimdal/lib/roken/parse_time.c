
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
#include "parse_time.h"

static struct units time_units[] = {
    {"year",	365 * 24 * 60 * 60},
    {"month",	30 * 24 * 60 * 60},
    {"week",	7 * 24 * 60 * 60},
    {"day",	24 * 60 * 60},
    {"hour",	60 * 60},
    {"h",	60 * 60},
    {"minute",	60},
    {"m",	60},
    {"second",	1},
    {"s",	1},
    {NULL, 0},
};

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
parse_time (const char *s, const char *def_unit)
{
    return parse_units (s, time_units, def_unit);
}

ROKEN_LIB_FUNCTION size_t ROKEN_LIB_CALL
unparse_time (int t, char *s, size_t len)
{
    return unparse_units (t, time_units, s, len);
}

ROKEN_LIB_FUNCTION size_t ROKEN_LIB_CALL
unparse_time_approx (int t, char *s, size_t len)
{
    return unparse_units_approx (t, time_units, s, len);
}

ROKEN_LIB_FUNCTION void ROKEN_LIB_CALL
print_time_table (FILE *f)
{
    print_units_table (time_units, f);
}