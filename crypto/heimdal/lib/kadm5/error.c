
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

#include "kadm5_locl.h"

RCSID("$Id$");

kadm5_ret_t
_kadm5_error_code(kadm5_ret_t code)
{
    switch(code){
    case HDB_ERR_EXISTS:
	return KADM5_DUP;
    case HDB_ERR_NOENTRY:
	return KADM5_UNK_PRINC;
    }
    return code;
}