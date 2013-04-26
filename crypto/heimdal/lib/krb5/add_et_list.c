
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

#include "krb5_locl.h"

/**
 * Add a specified list of error messages to the et list in context.
 * Call func (probably a comerr-generated function) with a pointer to
 * the current et_list.
 *
 * @param context A kerberos context.
 * @param func The generated com_err et function.
 *
 * @return Returns 0 to indicate success.  Otherwise an kerberos et
 * error code is returned, see krb5_get_error_message().
 *
 * @ingroup krb5
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_add_et_list (krb5_context context,
		  void (*func)(struct et_list **))
{
    (*func)(&context->et_list);
    return 0;
}