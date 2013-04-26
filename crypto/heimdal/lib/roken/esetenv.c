
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

#include "roken.h"

#include <err.h>

ROKEN_LIB_FUNCTION void ROKEN_LIB_CALL
esetenv(const char *var, const char *val, int rewrite)
{
    if (setenv (rk_UNCONST(var), rk_UNCONST(val), rewrite))
	errx (1, "failed setting environment variable %s", var);
}