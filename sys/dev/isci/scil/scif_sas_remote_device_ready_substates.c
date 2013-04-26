
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
 * @brief This file contains the entrance and exit methods for the ready
 *        sub-state machine states (OPERATIONAL, TASK_MGMT).
 */

#include <dev/isci/scil/scif_sas_remote_device.h>
#include <dev/isci/scil/scif_sas_domain.h>
#include <dev/isci/scil/scif_sas_logger.h>
#include <dev/isci/scil/scif_sas_internal_io_request.h>
#include <dev/isci/scil/scif_sas_controller.h>
#include <dev/isci/scil/sci_controller.h>

//******************************************************************************
//* P R O T E C T E D   M E T H O D S
//******************************************************************************

/**
 * @brief This method implements the actions taken when entering the
 *        READY OPERATIONAL substate.  This includes setting the state
 *        handler methods and issuing a scif_cb_remote_device_ready()
 *        notification to the user.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_REMOTE_DEVICE object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_remote_device_ready_operational_substate_enter(
   SCI_BASE_OBJECT_T *object
)
{
   SCIF_SAS_REMOTE_DEVICE_T * fw_device = (SCIF_SAS_REMOTE_DEVICE_T *)object;

   SET_STATE_HANDLER(
      fw_device,
      scif_sas_remote_device_ready_substate_handler_table,
      SCIF_SAS_REMOTE_DEVICE_READY_SUBSTATE_OPERATIONAL
   );

   SCIF_LOG_INFO((
      sci_base_object_get_logger(fw_device),
      SCIF_LOG_OBJECT_REMOTE_DEVICE | SCIF_LOG_OBJECT_REMOTE_DEVICE_CONFIG,
      "Domain:0x%x Device:0x%x device ready\n",
      fw_device->domain, fw_device
   ));

   // Notify the user that the device has become ready.
   scif_cb_remote_device_ready(
      fw_device->domain->controller, fw_device->domain, fw_device
   );
}

/**
 * @brief This method implements the actions taken when exiting the
 *        READY OPERATIONAL substate.  This method issues a
 *        scif_cb_remote_device_not_ready() notification to the framework
 *        user.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_REMOTE_DEVICE object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_remote_device_ready_operational_substate_exit(
   SCI_BASE_OBJECT_T *object
)
{
   SCIF_SAS_REMOTE_DEVICE_T * fw_device = (SCIF_SAS_REMOTE_DEVICE_T *)object;

   // Notify the user that the device has become ready.
   scif_cb_remote_device_not_ready(
      fw_device->domain->controller, fw_device->domain, fw_device
   );
}

/**
 * @brief This method implements the actions taken when entering the
 *        READY SUSPENDED substate.  This includes setting the state
 *        handler methods.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_REMOTE_DEVICE object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_remote_device_ready_suspended_substate_enter(
   SCI_BASE_OBJECT_T *object
)
{
   SCIF_SAS_REMOTE_DEVICE_T * fw_device = (SCIF_SAS_REMOTE_DEVICE_T *)object;

   SET_STATE_HANDLER(
      fw_device,
      scif_sas_remote_device_ready_substate_handler_table,
      SCIF_SAS_REMOTE_DEVICE_READY_SUBSTATE_SUSPENDED
   );
}

/**
 * @brief This method implements the actions taken when entering the
 *        READY TASK MGMT substate.  This includes setting the state
 *        handler methods.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_REMOTE_DEVICE object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_remote_device_ready_taskmgmt_substate_enter(
   SCI_BASE_OBJECT_T *object
)
{
   SCIF_SAS_REMOTE_DEVICE_T * fw_device = (SCIF_SAS_REMOTE_DEVICE_T *)object;

   SET_STATE_HANDLER(
      fw_device,
      scif_sas_remote_device_ready_substate_handler_table,
      SCIF_SAS_REMOTE_DEVICE_READY_SUBSTATE_TASK_MGMT
   );
}

/**
* @brief This method implements the actions taken when entering the
*        READY NCQ ERROR substate.  This includes setting the state
*        handler methods.
*
* @param[in]  object This parameter specifies the base object for which
*             the state transition is occurring.  This is cast into a
*             SCIF_SAS_REMOTE_DEVICE object in the method implementation.
*
* @return none
*/
static
void scif_sas_remote_device_ready_ncq_error_substate_enter(
   SCI_BASE_OBJECT_T *object
)
{
   SCIF_SAS_REMOTE_DEVICE_T         * fw_device = (SCIF_SAS_REMOTE_DEVICE_T *)object;
   SCI_STATUS                         status = SCI_SUCCESS;
   SCI_TASK_REQUEST_HANDLE_T          handle;
   SCIF_SAS_CONTROLLER_T            * fw_controller = fw_device->domain->controller;
   SCIF_SAS_TASK_REQUEST_T          * fw_task_request;
   SCIF_SAS_REQUEST_T               * fw_request;
   void                             * internal_task_memory;
   SCIF_SAS_DOMAIN_T                * fw_domain = fw_device->domain;
   SCI_FAST_LIST_ELEMENT_T          * pending_request_element;
   SCIF_SAS_REQUEST_T               * pending_request = NULL;

   SET_STATE_HANDLER(
      fw_device,
      scif_sas_remote_device_ready_substate_handler_table,
      SCIF_SAS_REMOTE_DEVICE_READY_SUBSTATE_NCQ_ERROR
   );

   internal_task_memory = scif_sas_controller_allocate_internal_request(fw_controller);
   ASSERT(internal_task_memory != NULL);

   fw_task_request = (SCIF_SAS_TASK_REQUEST_T*)internal_task_memory;

   fw_request = &fw_task_request->parent;

   //construct the scif io request
   status = scif_sas_internal_task_request_construct(
      fw_controller,
      fw_device,
      SCI_CONTROLLER_INVALID_IO_TAG,
      (void *)fw_task_request,
      &handle,
      SCI_SAS_ABORT_TASK_SET
   );

   pending_request_element = fw_domain->request_list.list_head;

   // Cycle through the fast list of IO requests.  Mark each request
   //  pending to this remote device so that they are not completed
   //  to the operating system when the request is terminated, but
   //  rather when the abort task set completes.
   while (pending_request_element != NULL)
   {
      pending_request =
         (SCIF_SAS_REQUEST_T*) sci_fast_list_get_object(pending_request_element);

      // The current element may be deleted from the list becasue of
      // IO completion so advance to the next element early
      pending_request_element = sci_fast_list_get_next(pending_request_element);

      if (pending_request->device == fw_device)
      {
         pending_request->is_waiting_for_abort_task_set = TRUE;
      }
   }

   scif_controller_start_task(
      fw_controller,
      fw_device,
      fw_request,
      SCI_CONTROLLER_INVALID_IO_TAG
   );
}

SCI_BASE_STATE_T scif_sas_remote_device_ready_substate_table
[SCIF_SAS_REMOTE_DEVICE_READY_SUBSTATE_MAX_STATES] =
{
   {
      SCIF_SAS_REMOTE_DEVICE_READY_SUBSTATE_OPERATIONAL,
      scif_sas_remote_device_ready_operational_substate_enter,
      scif_sas_remote_device_ready_operational_substate_exit
   },
   {
      SCIF_SAS_REMOTE_DEVICE_READY_SUBSTATE_SUSPENDED,
      scif_sas_remote_device_ready_suspended_substate_enter,
      NULL
   },
   {
      SCIF_SAS_REMOTE_DEVICE_READY_SUBSTATE_TASK_MGMT,
      scif_sas_remote_device_ready_taskmgmt_substate_enter,
      NULL
   },
   {
      SCIF_SAS_REMOTE_DEVICE_READY_SUBSTATE_NCQ_ERROR,
      scif_sas_remote_device_ready_ncq_error_substate_enter,
      NULL
   }
};