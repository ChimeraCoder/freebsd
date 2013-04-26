
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
 * @brief This file contains the implementation of the SCIF_SAS_REQUEST
 *        object.  The SCIF_SAS_REQUEST object provides data and methods
 *        that are common to both IO requests and task management requests.
 */


#include <dev/isci/scil/scic_controller.h>

#include <dev/isci/scil/scif_sas_request.h>
#include <dev/isci/scil/scif_sas_task_request.h>
#include <dev/isci/scil/scif_sas_controller.h>
#include <dev/isci/scil/scif_sas_domain.h>
#include <dev/isci/scil/scif_sas_remote_device.h>

#include <dev/isci/scil/scif_sas_logger.h>

//******************************************************************************
//* P R O T E C T E D   M E T H O D S
//******************************************************************************

/**
 * @brief This method constructs the SCIF_SAS_REQUEST object.
 *
 * @param[in] fw_request This parameter specifies the request object to
 *            be constructed.
 * @param[in] fw_device This parameter specifies the remote device object
 *            to which this request is destined.
 * @param[in] logger This parameter specifies the logger associated with
 *            this base request object.
 * @param[in] state_table This parameter specifies the table of state
 *            definitions to be utilized for the request state machine.
 *
 * @return none
 */
void scif_sas_request_construct(
   SCIF_SAS_REQUEST_T       * fw_request,
   SCIF_SAS_REMOTE_DEVICE_T * fw_device,
   SCI_BASE_LOGGER_T        * logger,
   SCI_BASE_STATE_T         * state_table
)
{
   sci_base_request_construct(&fw_request->parent, logger, state_table);

   SCIF_LOG_TRACE((
      sci_base_object_get_logger(fw_request),
      SCIF_LOG_OBJECT_TASK_MANAGEMENT,
      "scif_sas_request_construct(0x%x, 0x%x, 0x%x, 0x%x) enter\n",
      fw_request, fw_device, logger, state_table
   ));

   fw_request->device                    = fw_device;
   fw_request->is_internal               = FALSE;
   fw_request->lun                       = 0;
   fw_request->terminate_requestor       = NULL;
   fw_request->protocol_complete_handler = NULL;
   fw_request->is_high_priority          = FALSE;
   fw_request->is_waiting_for_abort_task_set = FALSE;

   sci_fast_list_element_init(fw_request, &fw_request->list_element);
}

/**
 * @brief This method will request the SCI core to terminate the supplied
 *        request.
 *
 * @param[in] fw_request This parameter specifies the request to be terminated.
 * @param[in] core_request This parameter specifies the core request (IO or
 *            task) to be terminated.
 *
 * @return This method returns the status of the core termination operation.
 */
SCI_STATUS scif_sas_request_terminate_start(
   SCIF_SAS_REQUEST_T      * fw_request,
   SCI_IO_REQUEST_HANDLE_T   core_request
)
{
   SCIF_LOG_TRACE((
      sci_base_object_get_logger(fw_request),
      SCIF_LOG_OBJECT_TASK_MANAGEMENT,
      "scif_sas_request_terminate_start(0x%x) enter\n",
      fw_request
   ));

   // Only increment the affected request count if this request is being
   // terminated at the behest of a task management request.
   if (fw_request->terminate_requestor != NULL)
      fw_request->terminate_requestor->affected_request_count++;

   return scic_controller_terminate_request(
             fw_request->device->domain->controller->core_object,
             fw_request->device->core_object,
             core_request
          );
}

/**
 * @brief This method will perform termination completion processing for
 *        the supplied request.  This includes updated the affected
 *        request count, if a task management request is what generated
 *        this termination.  Also, this method will attempt to transition
 *        to the READY OPERATIONAL state if this represents the last
 *        affected request.
 *
 * @param[in] fw_request This parameter specifies the request for which to
 *            perform termination completion processing.
 *
 * @return none
 */
void scif_sas_request_terminate_complete(
   SCIF_SAS_REQUEST_T * fw_request
)
{
   SCIF_LOG_TRACE((
      sci_base_object_get_logger(fw_request),
      SCIF_LOG_OBJECT_TASK_MANAGEMENT,
      "scif_sas_request_terminate_complete(0x%x) enter\n",
      fw_request
   ));

   // For requests that were terminated due to a task management request,
   // check to see if the task management request has completed.
   if (fw_request->terminate_requestor != NULL)
      scif_sas_task_request_operation_complete(fw_request->terminate_requestor);
}