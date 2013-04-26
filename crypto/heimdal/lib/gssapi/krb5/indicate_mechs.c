
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

#include "gsskrb5_locl.h"

OM_uint32 GSSAPI_CALLCONV _gsskrb5_indicate_mechs
           (OM_uint32 * minor_status,
            gss_OID_set * mech_set
           )
{
  OM_uint32 ret, junk;

  ret = gss_create_empty_oid_set(minor_status, mech_set);
  if (ret)
      return ret;

  ret = gss_add_oid_set_member(minor_status, GSS_KRB5_MECHANISM, mech_set);
  if (ret) {
      gss_release_oid_set(&junk, mech_set);
      return ret;
  }

  *minor_status = 0;
  return GSS_S_COMPLETE;
}