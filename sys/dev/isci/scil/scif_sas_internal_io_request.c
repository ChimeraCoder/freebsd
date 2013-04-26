
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
 * @brief This file contains the implementation of the
 *        SCIF_SAS_INTERNAL_IO_REQUEST object.
 */


#include <dev/isci/scil/scic_io_request.h>
#include <dev/isci/scil/scic_remote_device.h>
#include <dev/isci/scil/scic_user_callback.h>
#include <dev/isci/scil/scic_controller.h>
#include <dev/isci/scil/scic_task_request.h>
#include <dev/isci/scil/scif_user_callback.h>

#include <dev/isci/scil/scif_sas_controller.h>
#include <dev/isci/scil/scif_sas_domain.h>
#include <dev/isci/scil/scif_sas_remote_device.h>
#include <dev/isci/scil/scif_sas_io_request.h>
#include <dev/isci/scil/scif_sas_internal_io_request.h>
#include <dev/isci/scil/scif_sas_task_request.h>
#include <dev/isci/scil/scif_sas_stp_io_request.h>
#include <dev/isci/scil/scif_sas_logger.h>
#include <dev/isci/scil/scif_sas_smp_io_request.h>
#include <dev/isci/scil/sci_util.h>

//******************************************************************************
//* P U B L I C   M E T H O D S
//******************************************************************************

/**
 * @brief this routine return all memory needed for an internal request, both
 *        framework and core request.
 *
 * @return U32 size of all memory needed for an internal request
 */
U32 scif_sas_internal_request_get_object_size(
   void
)
{
   return MAX(
            (sizeof(SCIF_SAS_INTERNAL_IO_REQUEST_T) + scic_io_request_get_object_size()),
            (sizeof(SCIF_SAS_TASK_REQUEST_T) + scic_task_request_get_object_size())
             );
}


/**
 * @brief This method constructs an internal smp request.
 *
 * @param[in] fw_controller The framework controller
 * @param[in] fw_device The smp device that the internal io targets to.
 * @param[in] internal_io_memory The memory space for the internal io.
 * @param[in] io_tag The io tag for the internl io to be constructed.
 * @param[in] smp_command A pointer to the smp request data structure according
 *       to SAS protocol.
 *
 * @return Indicate if the internal io was successfully constructed.
 * @retval SCI_SUCCESS This value is returned if the internal io was
 *         successfully constructed.
 * @retval SCI_FAILURE This value is returned if the internal io was failed to
 *         be constructed.
 */
SCI_STATUS scif_sas_internal_io_request_construct_smp(
   SCIF_SAS_CONTROLLER_T       * fw_controller,
   SCIF_SAS_REMOTE_DEVICE_T    * fw_device,
   void                        * internal_io_memory,
   U16                           io_tag,
   SMP_REQUEST_T               * smp_command
)
{
   SCIF_SAS_INTERNAL_IO_REQUEST_T * fw_internal_io  =
     (SCIF_SAS_INTERNAL_IO_REQUEST_T*)internal_io_memory;

   SCIF_SAS_IO_REQUEST_T * fw_io =
     (SCIF_SAS_IO_REQUEST_T*)internal_io_memory;

   SCI_STATUS status;

   //call common smp request construct routine.
   status = scif_sas_io_request_construct_smp(
               fw_controller,
               fw_device,
               internal_io_memory,
               (char *)internal_io_memory + sizeof(SCIF_SAS_INTERNAL_IO_REQUEST_T),
               SCI_CONTROLLER_INVALID_IO_TAG,
               smp_command,
               NULL //there is no associated user io object.
            );

   //Codes below are all internal io related.
   if (status == SCI_SUCCESS)
   {
      //set the is_internal flag
      fw_io->parent.is_internal = TRUE;

      if (fw_internal_io->internal_io_timer == NULL)
      {
         //create the timer for this internal request.
         fw_internal_io->internal_io_timer =
            scif_cb_timer_create(
               (SCI_CONTROLLER_HANDLE_T *)fw_controller,
               scif_sas_internal_io_request_timeout_handler,
               (void*)fw_io
            );
      }
      else
      {
         ASSERT (0);
      }

      //insert into high priority queue
      if ( !sci_pool_full(fw_controller->hprq.pool) )
      {
         sci_pool_put(
            fw_controller->hprq.pool, (POINTER_UINT) internal_io_memory
         );
      }
      else
      {
         SCIF_LOG_ERROR((
            sci_base_object_get_logger(fw_controller),
            SCIF_LOG_OBJECT_CONTROLLER | SCIF_LOG_OBJECT_REMOTE_DEVICE,
            "scif_sas_internal_io_request_construct_smp, high priority queue full!\n"
         ));

         scif_sas_internal_io_request_destruct(fw_controller, fw_internal_io);

         //return failure status.
         return SCI_FAILURE_INSUFFICIENT_RESOURCES;
      }
   }

   return status;
}


/**
 * @brief This method constructs an internal smp request.
 * @param[in] fw_io
 *
 * @return SCI_STATUS
 */
SCI_STATUS scif_sas_internal_io_request_construct_stp(
   SCIF_SAS_INTERNAL_IO_REQUEST_T * fw_io
)
{
   //TBD
   return SCI_SUCCESS;
}


/**
 * @brief This method handles the timeout situation for an internal io.
 *
 * @param[in] fw_internal_io The timed out IO.
 *
 * @return none
 */
void scif_sas_internal_io_request_timeout_handler(
   void * fw_internal_io
)
{
   SCIF_SAS_REQUEST_T * fw_request = (SCIF_SAS_REQUEST_T *)fw_internal_io;

   SCIF_LOG_TRACE((
      sci_base_object_get_logger(fw_request),
      SCIF_LOG_OBJECT_IO_REQUEST,
      "scif_sas_internal_io_request_timeout_handler(0x%x) enter\n",
      fw_internal_io
   ));

   fw_request->state_handlers->abort_handler(&fw_request->parent);
}


/**
 * @brief This methods takes care of completion of an internal request about its
 *        "internal" related feature, including the memory recycling and timer.
 *
 * @param[in] fw_controller The framework controller object.
 * @param[in] fw_internal_io The internal io to be completed.
 * @param[in] completion_status the completeion status by core and framework so
 *       far.
 *
 * @return none
 */
void scif_sas_internal_io_request_complete(
   SCIF_SAS_CONTROLLER_T          * fw_controller,
   SCIF_SAS_INTERNAL_IO_REQUEST_T * fw_internal_io,
   SCI_STATUS                       completion_status
)
{
   SCIF_LOG_TRACE((
      sci_base_object_get_logger(fw_controller),
      SCIF_LOG_OBJECT_IO_REQUEST,
      "scif_sas_internal_io_request_complete(0x%x, 0x%x, 0x%x) enter\n",
       fw_controller, fw_internal_io, completion_status
   ));

   scif_cb_timer_stop(fw_controller, fw_internal_io->internal_io_timer);
   scif_sas_internal_io_request_destruct(fw_controller, fw_internal_io);
}


/**
 * @brief This methods takes care of destruction of an internal request about its
 *        "internal" related feature, including the memory recycling and timer.
 *
 * @param[in] fw_controller The framework controller object.
 * @param[in] fw_internal_io The internal io to be completed.
 *
 * @return none
 */
void scif_sas_internal_io_request_destruct(
   SCIF_SAS_CONTROLLER_T          * fw_controller,
   SCIF_SAS_INTERNAL_IO_REQUEST_T * fw_internal_io
)
{
   if (fw_internal_io->internal_io_timer != NULL)
   {
      scif_cb_timer_destroy(fw_controller, fw_internal_io->internal_io_timer);
      fw_internal_io->internal_io_timer = NULL;
   }
   scif_sas_controller_free_internal_request(fw_controller, fw_internal_io);
}