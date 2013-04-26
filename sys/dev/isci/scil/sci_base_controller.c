
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

/**
 * @file
 *
 * @brief This file contains the base controller method implementations and
 *        any constants or structures private to the base controller object
 *        or common to all controller derived objects.
 */

#include <dev/isci/scil/sci_base_controller.h>
#include <dev/isci/scil/sci_controller.h>

//******************************************************************************
//* P U B L I C   M E T H O D S
//******************************************************************************

SCI_MEMORY_DESCRIPTOR_LIST_HANDLE_T sci_controller_get_memory_descriptor_list_handle(
   SCI_CONTROLLER_HANDLE_T controller
)
{
   SCI_BASE_CONTROLLER_T * this_controller = (SCI_BASE_CONTROLLER_T*)controller;
   return (SCI_MEMORY_DESCRIPTOR_LIST_HANDLE_T) &this_controller->mdl;
}

//******************************************************************************
//* P R O T E C T E D   M E T H O D S
//******************************************************************************

void sci_base_controller_construct(
   SCI_BASE_CONTROLLER_T               * this_controller,
   SCI_BASE_LOGGER_T                   * logger,
   SCI_BASE_STATE_T                    * state_table,
   SCI_PHYSICAL_MEMORY_DESCRIPTOR_T    * mdes,
   U32                                   mde_count,
   SCI_MEMORY_DESCRIPTOR_LIST_HANDLE_T   next_mdl
)
{
   sci_base_object_construct((SCI_BASE_OBJECT_T *)this_controller, logger);

   sci_base_state_machine_construct(
      &this_controller->state_machine,
      &this_controller->parent,
      state_table,
      SCI_BASE_CONTROLLER_STATE_INITIAL
   );

   sci_base_mdl_construct(&this_controller->mdl, mdes, mde_count, next_mdl);

   sci_base_state_machine_start(&this_controller->state_machine);
}