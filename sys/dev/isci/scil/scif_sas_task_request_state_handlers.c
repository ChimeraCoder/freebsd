
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
 *        to the framework io request state handler methods.
 */

#include <dev/isci/scil/scif_sas_logger.h>
#include <dev/isci/scil/scif_sas_task_request.h>

//******************************************************************************
//* C O N S T R U C T E D   H A N D L E R S
//******************************************************************************

/**
 * @brief This method provides CONSTRUCTED state specific handling for
 *        when the user attempts to start the supplied task request.
 *
 * @param[in] task_request This parameter specifies the task request object
 *            to be started.
 *
 * @return This method returns a value indicating if the task request was
 *         successfully started or not.
 * @retval SCI_SUCCESS This return value indicates successful starting
 *         of the task request.
 */
static
SCI_STATUS scif_sas_task_request_constructed_start_handler(
   SCI_BASE_REQUEST_T * task_request
)
{
   sci_base_state_machine_change_state(
      &task_request->state_machine, SCI_BASE_REQUEST_STATE_STARTED
   );

   return SCI_SUCCESS;
}

/**
 * @brief This method provides CONSTRUCTED state specific handling for
 *        when the user attempts to abort the supplied task request.
 *
 * @param[in] task_request This parameter specifies the task request object
 *            to be aborted.
 *
 * @return This method returns a value indicating if the task request was
 *         successfully aborted or not.
 * @retval SCI_SUCCESS This return value indicates successful aborting
 *         of the task request.
 */
static
SCI_STATUS scif_sas_task_request_constructed_abort_handler(
   SCI_BASE_REQUEST_T * task_request
)
{
   sci_base_state_machine_change_state(
      &task_request->state_machine, SCI_BASE_REQUEST_STATE_COMPLETED
   );

   return SCI_SUCCESS;
}

//******************************************************************************
//* S T A R T E D   H A N D L E R S
//******************************************************************************

/**
 * @brief This method provides STARTED state specific handling for
 *        when the user attempts to abort the supplied task request.
 *
 * @param[in] task_request This parameter specifies the task request object
 *            to be aborted.
 *
 * @return This method returns a value indicating if the aborting the
 *         task request was successfully started.
 * @retval SCI_SUCCESS This return value indicates that the abort process
 *         began successfully.
 */
static
SCI_STATUS scif_sas_task_request_started_abort_handler(
   SCI_BASE_REQUEST_T * task_request
)
{
   SCIF_SAS_REQUEST_T * fw_request = (SCIF_SAS_REQUEST_T *) task_request;

   sci_base_state_machine_change_state(
      &task_request->state_machine, SCI_BASE_REQUEST_STATE_ABORTING
   );

   return fw_request->status;
}

/**
 * @brief This method provides STARTED state specific handling for
 *        when the user attempts to complete the supplied task request.
 *
 * @param[in] task_request This parameter specifies the task request object
 *            to be completed.
 *
 * @return This method returns a value indicating if the completion of the
 *         task request was successful.
 * @retval SCI_SUCCESS This return value indicates that the completion process
 *         was successful.
 */
static
SCI_STATUS scif_sas_task_request_started_complete_handler(
   SCI_BASE_REQUEST_T * task_request
)
{
   sci_base_state_machine_change_state(
      &task_request->state_machine, SCI_BASE_REQUEST_STATE_COMPLETED
   );

   return SCI_SUCCESS;
}

//******************************************************************************
//* C O M P L E T E D   H A N D L E R S
//******************************************************************************

/**
 * @brief This method provides COMPLETED state specific handling for
 *        when the user attempts to destruct the supplied task request.
 *
 * @param[in] task_request This parameter specifies the task request object
 *            to be destructed.
 *
 * @return This method returns a value indicating if the destruct
 *         operation was successful.
 * @retval SCI_SUCCESS This return value indicates that the destruct
 *         was successful.
 */
static
SCI_STATUS scif_sas_task_request_completed_destruct_handler(
   SCI_BASE_REQUEST_T * task_request
)
{
   SCIF_SAS_REQUEST_T * fw_request = (SCIF_SAS_REQUEST_T *)task_request;

   sci_base_state_machine_change_state(
      &task_request->state_machine, SCI_BASE_REQUEST_STATE_FINAL
   );

   sci_base_state_machine_logger_deinitialize(
      &task_request->state_machine_logger,
      &task_request->state_machine
   );

   if (fw_request->is_internal == TRUE)
   {
      scif_sas_internal_task_request_destruct(
         (SCIF_SAS_TASK_REQUEST_T *)fw_request
      );
   }

   return SCI_SUCCESS;
}

//******************************************************************************
//* A B O R T I N G   H A N D L E R S
//******************************************************************************

/**
 * @brief This method provides ABORTING state specific handling for
 *        when the user attempts to complete the supplied task request.
 *
 * @param[in] task_request This parameter specifies the task request object
 *            to be completed.
 *
 * @return This method returns a value indicating if the completion
 *         operation was successful.
 * @retval SCI_SUCCESS This return value indicates that the completion
 *         was successful.
 */
static
SCI_STATUS scif_sas_task_request_aborting_complete_handler(
   SCI_BASE_REQUEST_T * task_request
)
{
   sci_base_state_machine_change_state(
      &task_request->state_machine, SCI_BASE_REQUEST_STATE_COMPLETED
   );

   return SCI_SUCCESS;
}

//******************************************************************************
//* D E F A U L T   H A N D L E R S
//******************************************************************************

/**
 * @brief This method provides DEFAULT handling for when the user
 *        attempts to start the supplied task request.
 *
 * @param[in] task_request This parameter specifies the task request object
 *            to be started.
 *
 * @return This method returns an indication that the start operation is
 *         not allowed.
 * @retval SCI_FAILURE_INVALID_STATE This value is always returned.
 */
static
SCI_STATUS scif_sas_task_request_default_start_handler(
   SCI_BASE_REQUEST_T * task_request
)
{
   SCIF_LOG_ERROR((
      sci_base_object_get_logger((SCIF_SAS_TASK_REQUEST_T *) task_request),
      SCIF_LOG_OBJECT_TASK_MANAGEMENT,
      "TaskRequest:0x%x State:0x%x invalid state to start\n",
      task_request,
      sci_base_state_machine_get_state(
         &((SCIF_SAS_TASK_REQUEST_T *) task_request)->parent.parent.state_machine)
   ));

   return SCI_FAILURE_INVALID_STATE;
}

/**
 * @brief This method provides DEFAULT handling for when the user
 *        attempts to abort the supplied task request.
 *
 * @param[in] task_request This parameter specifies the task request object
 *            to be aborted.
 *
 * @return This method returns an indication that the abort operation is
 *         not allowed.
 * @retval SCI_FAILURE_INVALID_STATE This value is always returned.
 */
static
SCI_STATUS scif_sas_task_request_default_abort_handler(
   SCI_BASE_REQUEST_T * task_request
)
{
   SCIF_LOG_ERROR((
      sci_base_object_get_logger((SCIF_SAS_TASK_REQUEST_T *) task_request),
      SCIF_LOG_OBJECT_TASK_MANAGEMENT,
      "TaskRequest:0x%x State:0x%x invalid state to abort\n",
      task_request,
      sci_base_state_machine_get_state(
         &((SCIF_SAS_TASK_REQUEST_T *) task_request)->parent.parent.state_machine)
   ));

   return SCI_FAILURE_INVALID_STATE;
}

/**
 * @brief This method provides DEFAULT handling for when the user
 *        attempts to complete the supplied task request.
 *
 * @param[in] task_request This parameter specifies the task request object
 *            to be completed.
 *
 * @return This method returns an indication that complete operation is
 *         not allowed.
 * @retval SCI_FAILURE_INVALID_STATE This value is always returned.
 */
static
SCI_STATUS scif_sas_task_request_default_complete_handler(
   SCI_BASE_REQUEST_T * task_request
)
{
   SCIF_LOG_ERROR((
      sci_base_object_get_logger((SCIF_SAS_TASK_REQUEST_T *) task_request),
      SCIF_LOG_OBJECT_TASK_MANAGEMENT,
      "TaskRequest:0x%x State:0x%x invalid state to complete\n",
      task_request,
      sci_base_state_machine_get_state(
         &((SCIF_SAS_TASK_REQUEST_T *) task_request)->parent.parent.state_machine)
   ));

   return SCI_FAILURE_INVALID_STATE;
}

/**
 * @brief This method provides DEFAULT handling for when the user
 *        attempts to destruct the supplied task request.
 *
 * @param[in] task_request This parameter specifies the task request object
 *            to be destructed.
 *
 * @return This method returns an indication that destruct operation is
 *         not allowed.
 * @retval SCI_FAILURE_INVALID_STATE This value is always returned.
 */
static
SCI_STATUS scif_sas_task_request_default_destruct_handler(
   SCI_BASE_REQUEST_T * task_request
)
{
   SCIF_LOG_ERROR((
      sci_base_object_get_logger((SCIF_SAS_TASK_REQUEST_T *) task_request),
      SCIF_LOG_OBJECT_TASK_MANAGEMENT,
      "TaskRequest:0x%x State:0x%x invalid state to destruct.\n",
      task_request,
      sci_base_state_machine_get_state(
         &((SCIF_SAS_TASK_REQUEST_T *) task_request)->parent.parent.state_machine)
   ));

   return SCI_FAILURE_INVALID_STATE;
}


SCI_BASE_REQUEST_STATE_HANDLER_T scif_sas_task_request_state_handler_table[] =
{
   // SCI_BASE_REQUEST_STATE_INITIAL
   {
      scif_sas_task_request_default_start_handler,
      scif_sas_task_request_default_abort_handler,
      scif_sas_task_request_default_complete_handler,
      scif_sas_task_request_default_destruct_handler
   },
   // SCI_BASE_REQUEST_STATE_CONSTRUCTED
   {
      scif_sas_task_request_constructed_start_handler,
      scif_sas_task_request_constructed_abort_handler,
      scif_sas_task_request_default_complete_handler,
      scif_sas_task_request_default_destruct_handler
   },
   // SCI_BASE_REQUEST_STATE_STARTED
   {
      scif_sas_task_request_default_start_handler,
      scif_sas_task_request_started_abort_handler,
      scif_sas_task_request_started_complete_handler,
      scif_sas_task_request_default_destruct_handler
   },
   // SCI_BASE_REQUEST_STATE_COMPLETED
   {
      scif_sas_task_request_default_start_handler,
      scif_sas_task_request_default_abort_handler,
      scif_sas_task_request_default_complete_handler,
      scif_sas_task_request_completed_destruct_handler
   },
   // SCI_BASE_REQUEST_STATE_ABORTING
   {
      scif_sas_task_request_default_start_handler,
      scif_sas_task_request_default_abort_handler,
      scif_sas_task_request_aborting_complete_handler,
      scif_sas_task_request_default_destruct_handler
   },
   // SCI_BASE_REQUEST_STATE_FINAL
   {
      scif_sas_task_request_default_start_handler,
      scif_sas_task_request_default_abort_handler,
      scif_sas_task_request_default_complete_handler,
      scif_sas_task_request_default_destruct_handler
   },
};