
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

OM_uint32 _gsskrb5_release_buffer
           (OM_uint32 * minor_status,
            gss_buffer_t buffer
           )
{
  *minor_status = 0;
  free (buffer->value);
  buffer->value  = NULL;
  buffer->length = 0;
  return GSS_S_COMPLETE;
}