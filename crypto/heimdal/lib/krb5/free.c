
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

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_free_kdc_rep(krb5_context context, krb5_kdc_rep *rep)
{
    free_KDC_REP(&rep->kdc_rep);
    free_EncTGSRepPart(&rep->enc_part);
    free_KRB_ERROR(&rep->error);
    memset(rep, 0, sizeof(*rep));
    return 0;
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_xfree (void *ptr)
{
    free (ptr);
    return 0;
}