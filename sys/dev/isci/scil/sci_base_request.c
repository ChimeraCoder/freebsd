
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <dev/isci/scil/sci_base_request.h>

void sci_base_request_construct(
   SCI_BASE_REQUEST_T *this_request,
   SCI_BASE_LOGGER_T  *my_logger,
   SCI_BASE_STATE_T   *my_state_table
)
{
   sci_base_object_construct(
      &this_request->parent,
      my_logger
   );

   sci_base_state_machine_construct(
      &this_request->state_machine,
      &this_request->parent,
      my_state_table,
      SCI_BASE_REQUEST_STATE_INITIAL
   );

   sci_base_state_machine_start(
      &this_request->state_machine
   );
}