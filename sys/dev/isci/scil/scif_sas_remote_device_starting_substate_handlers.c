
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
 * @brief This file contains all of the method implementations pertaining
 *        to the framework remote device STARTING sub-state handler methods.
 *        The STARTING sub-state machine is responsible for ensuring that
 *        all initialization and configuration for a particular remote
 *        device is complete before transitioning to the READY state
 *        (i.e. before allowing normal host IO).
 */

#include <dev/isci/scil/scic_remote_device.h>

#include <dev/isci/scil/scif_sas_logger.h>
#include <dev/isci/scil/scif_sas_remote_device.h>
#include <dev/isci/scil/scif_sas_domain.h>
#include <dev/isci/scil/scif_sas_task_request.h>

//******************************************************************************
//* G E N E R A L   S T O P   H A N D L E R S
//******************************************************************************

/**
 * @brief This method provides startig sub-state specific handling for
 *        when the remote device is requested to stop.  This will occur
 *        when there is a link failure during the starting operation.
 *
 * @param[in]  remote_device This parameter specifies the remote device
 *             object for which the failure condition occurred.
 *
 * @return This method returns an indication as to whether the failure
 *         operation completed successfully.
 */
static
SCI_STATUS
scif_sas_remote_device_starting_state_general_stop_handler(
   SCI_BASE_REMOTE_DEVICE_T * remote_device
)
{
   SCIF_SAS_REMOTE_DEVICE_T * fw_device = (SCIF_SAS_REMOTE_DEVICE_T *)
                                          remote_device;

   SCIF_LOG_INFO((
      sci_base_object_get_logger(fw_device),
      SCIF_LOG_OBJECT_REMOTE_DEVICE,
      "RemoteDevice:0x%x starting device requested to stop\n",
      fw_device
   ));

   fw_device->domain->device_start_in_progress_count--;

   sci_base_state_machine_change_state(
      &fw_device->parent.state_machine, SCI_BASE_REMOTE_DEVICE_STATE_STOPPING
   );

   return SCI_SUCCESS;
}

//******************************************************************************
//* A W A I T   C O M P L E T E   H A N D L E R S
//******************************************************************************

/**
 * @brief This method provides AWAIT START COMPLETE sub-state specific
 *        handling for when the remote device undergoes a failure
 *        condition.
 *
 * @param[in]  remote_device This parameter specifies the remote device
 *             object for which the failure condition occurred.
 *
 * @return This method returns an indication as to whether the failure
 *         operation completed successfully.
 */
static
SCI_STATUS scif_sas_remote_device_starting_await_complete_fail_handler(
   SCI_BASE_REMOTE_DEVICE_T * remote_device
)
{
   SCIF_SAS_REMOTE_DEVICE_T * fw_device = (SCIF_SAS_REMOTE_DEVICE_T *)
                                          remote_device;

   SCIF_LOG_WARNING((
      sci_base_object_get_logger(fw_device),
      SCIF_LOG_OBJECT_REMOTE_DEVICE,
      "RemoteDevice:0x%x starting device failed, start complete not received\n",
      fw_device
   ));

   sci_base_state_machine_change_state(
      &fw_device->parent.state_machine, SCI_BASE_REMOTE_DEVICE_STATE_FAILED
   );

   return SCI_SUCCESS;
}

/**
 * @brief This method provides AWAIT COMPLETE state specific handling for
 *        when the core remote device object issues a device not ready
 *        notification.  In the AWAIT COMPLETE state we do not inform
 *        the framework user of the state change of the device, since the
 *        user is unaware of the remote device start process.
 *
 * @param[in]  remote_device This parameter specifies the remote device
 *             object for which the notification occurred.
 *
 * @return none.
 */
static
void scif_sas_remote_device_starting_await_complete_not_ready_handler(
   SCIF_SAS_REMOTE_DEVICE_T * fw_device,
   U32                        reason_code
)
{
}

/**
 * @brief This method provides AWAIT START COMPLETE sub-state specific
 *        handling for when the core provides a start complete notification
 *        for the remote device.  If the start completion status indicates
 *        a successful start, then the device is transitioned into the
 *        READY state.  All other status cause a transition to the
 *        FAILED state and a scif_cb_controller_error() notification
 *        message to the framework user.
 *
 * @param[in]  fw_device This parameter specifies the remote device
 *             object for which the notification has occurred.
 *
 * @return none.
 */
static
void scif_sas_remote_device_starting_await_complete_start_complete_handler(
   SCIF_SAS_REMOTE_DEVICE_T * fw_device,
   SCI_STATUS                 completion_status
)
{
   if (completion_status == SCI_SUCCESS)
   {
      /** @todo need to add support for resetting the device first.  This can
                wait until 1.3. */
      /** @todo Update to comprehend situations (i.e. SATA) where config is
                needed. */

      sci_base_state_machine_change_state(
         &fw_device->starting_substate_machine,
         SCIF_SAS_REMOTE_DEVICE_STARTING_SUBSTATE_AWAIT_READY
      );
   }
   else
   {
      SCIF_LOG_WARNING((
         sci_base_object_get_logger(fw_device),
         SCIF_LOG_OBJECT_REMOTE_DEVICE | SCIF_LOG_OBJECT_REMOTE_DEVICE_CONFIG,
         "Device:0x%x Status:0x%x failed to start core device\n",
         fw_device
      ));

      sci_base_state_machine_change_state(
         &fw_device->parent.state_machine,
         SCI_BASE_REMOTE_DEVICE_STATE_FAILED
      );

      // Something is seriously wrong.  Starting the core remote device
      // shouldn't fail in anyway in this state.
      scif_cb_controller_error(fw_device->domain->controller,
              SCI_CONTROLLER_REMOTE_DEVICE_ERROR);
   }
}

//******************************************************************************
//* C O M P L E T E   H A N D L E R S
//******************************************************************************

/**
 * @brief This method provides STARTING AWAIT READY sub-state specific
 *        handling for when the core provides a device ready notification
 *        for the remote device.  This essentially, causes a transition
 *        of the framework remote device into the READY state.
 *
 * @param[in]  fw_device This parameter specifies the remote device
 *             object for which the notification has occurred.
 *
 * @return none.
 */
static
void scif_sas_remote_device_starting_await_ready_ready_handler(
   SCIF_SAS_REMOTE_DEVICE_T * fw_device
)
{
#if !defined(DISABLE_WIDE_PORTED_TARGETS)
   if (fw_device->destination_state ==
          SCIF_SAS_REMOTE_DEVICE_DESTINATION_STATE_UPDATING_PORT_WIDTH)
   {
      {
         sci_base_state_machine_change_state(
            &fw_device->parent.state_machine,
            SCI_BASE_REMOTE_DEVICE_STATE_UPDATING_PORT_WIDTH
         );
      }
   }
   else
#endif
   {
      sci_base_state_machine_change_state(
         &fw_device->parent.state_machine, SCI_BASE_REMOTE_DEVICE_STATE_READY
      );
   }

#if !defined(DISABLE_WIDE_PORTED_TARGETS)
   scif_sas_domain_remote_device_start_complete(fw_device->domain,fw_device);
#endif
}


SCIF_SAS_REMOTE_DEVICE_STATE_HANDLER_T
scif_sas_remote_device_starting_substate_handler_table[] =
{
   // SCIF_SAS_REMOTE_DEVICE_STARTING_SUBSTATE_AWAIT_COMPLETE
   {
      {
         scif_sas_remote_device_default_start_handler,
         scif_sas_remote_device_starting_state_general_stop_handler,
         scif_sas_remote_device_starting_await_complete_fail_handler,
         scif_sas_remote_device_default_destruct_handler,
         scif_sas_remote_device_default_reset_handler,
         scif_sas_remote_device_default_reset_complete_handler,
         scif_sas_remote_device_default_start_io_handler,
         scif_sas_remote_device_default_complete_io_handler,
         scif_sas_remote_device_default_continue_io_handler,
         scif_sas_remote_device_default_start_task_handler,
         scif_sas_remote_device_default_complete_task_handler
      },
      scif_sas_remote_device_starting_await_complete_start_complete_handler,
      scif_sas_remote_device_default_stop_complete_handler,
      scif_sas_remote_device_default_ready_handler,
      scif_sas_remote_device_starting_await_complete_not_ready_handler,
      scif_sas_remote_device_default_start_io_handler,
      scif_sas_remote_device_default_complete_high_priority_io_handler
   },
   // SCIF_SAS_REMOTE_DEVICE_STARTING_SUBSTATE_AWAIT_READY
   {
      {
         scif_sas_remote_device_default_start_handler,
         scif_sas_remote_device_starting_state_general_stop_handler,
         scif_sas_remote_device_starting_await_complete_fail_handler,
         scif_sas_remote_device_default_destruct_handler,
         scif_sas_remote_device_default_reset_handler,
         scif_sas_remote_device_default_reset_complete_handler,
         scif_sas_remote_device_default_start_io_handler,
         scif_sas_remote_device_default_complete_io_handler,
         scif_sas_remote_device_default_continue_io_handler,
         scif_sas_remote_device_default_start_task_handler,
         scif_sas_remote_device_default_complete_task_handler
      },
      scif_sas_remote_device_default_start_complete_handler,
      scif_sas_remote_device_default_stop_complete_handler,
      scif_sas_remote_device_starting_await_ready_ready_handler,
      scif_sas_remote_device_default_not_ready_handler,
      scif_sas_remote_device_default_start_io_handler,
      scif_sas_remote_device_default_complete_high_priority_io_handler
   }
};