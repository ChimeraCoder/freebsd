
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

#include <dev/isci/scil/sci_base_remote_device.h>

void sci_base_remote_device_construct(
   SCI_BASE_REMOTE_DEVICE_T *this_device,
   SCI_BASE_LOGGER_T        *logger,
   SCI_BASE_STATE_T         *state_table
)
{
   sci_base_object_construct(
      &this_device->parent,
      logger
   );

   sci_base_state_machine_construct(
      &this_device->state_machine,
      &this_device->parent,
      state_table,
      SCI_BASE_REMOTE_DEVICE_STATE_INITIAL
   );

   sci_base_state_machine_start(
      &this_device->state_machine
   );
}