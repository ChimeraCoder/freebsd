
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
 * @brief This file contains the entrance and exit methods for the starting
 *        sub-state machine states (AWAIT COMPELTE).  The starting sub-state
 *        machine manages the steps necessary to initialize and configure
 *        a remote device.
 */

#include <dev/isci/scil/scif_sas_remote_device.h>
#include <dev/isci/scil/scif_sas_domain.h>
#include <dev/isci/scil/scif_sas_logger.h>


//******************************************************************************
//* P R O T E C T E D   M E T H O D S
//******************************************************************************

/**
 * @brief This method implements the actions taken when entering the
 *        STARTING AWAIT COMPLETE substate.  This includes setting the
 *        state handler methods.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_REMOTE_DEVICE object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_remote_device_starting_await_complete_substate_enter(
   SCI_BASE_OBJECT_T *object
)
{
   SCIF_SAS_REMOTE_DEVICE_T * fw_device = (SCIF_SAS_REMOTE_DEVICE_T *)object;

   SET_STATE_HANDLER(
      fw_device,
      scif_sas_remote_device_starting_substate_handler_table,
      SCIF_SAS_REMOTE_DEVICE_STARTING_SUBSTATE_AWAIT_COMPLETE
   );

   fw_device->domain->device_start_in_progress_count++;
   fw_device->domain->device_start_count++;
}

/**
 * @brief This method implements the actions taken when entering the
 *        STARTING COMPLETE substate.  This includes setting the
 *        state handler methods.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_REMOTE_DEVICE object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_remote_device_starting_complete_substate_enter(
   SCI_BASE_OBJECT_T *object
)
{
   SCIF_SAS_REMOTE_DEVICE_T * fw_device = (SCIF_SAS_REMOTE_DEVICE_T *)object;

   SET_STATE_HANDLER(
      fw_device,
      scif_sas_remote_device_starting_substate_handler_table,
      SCIF_SAS_REMOTE_DEVICE_STARTING_SUBSTATE_AWAIT_READY
   );
}


SCI_BASE_STATE_T
scif_sas_remote_device_starting_substate_table
[SCIF_SAS_REMOTE_DEVICE_STARTING_SUBSTATE_MAX_STATES] =
{
   {
      SCIF_SAS_REMOTE_DEVICE_STARTING_SUBSTATE_AWAIT_COMPLETE,
      scif_sas_remote_device_starting_await_complete_substate_enter,
      NULL
   },
   {
      SCIF_SAS_REMOTE_DEVICE_STARTING_SUBSTATE_AWAIT_READY,
      scif_sas_remote_device_starting_complete_substate_enter,
      NULL
   }
};