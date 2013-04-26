
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
 * @brief This file contains all of the SCIF_SAS_IO_REQUEST object
 *        state entrance and exit method implementations.
 */

#include <dev/isci/scil/scic_controller.h>

#include <dev/isci/scil/scif_sas_io_request.h>
#include <dev/isci/scil/scif_sas_domain.h>
#include <dev/isci/scil/scif_sas_controller.h>
#include <dev/isci/scil/scif_sas_logger.h>

//******************************************************************************
//* P R O T E C T E D   M E T H O D S
//******************************************************************************

/**
 * @brief This method implements the actions taken when entering the
 *        INITIAL state.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_IO_REQUEST object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_io_request_initial_state_enter(
   SCI_BASE_OBJECT_T *object
)
{
   SCIF_SAS_IO_REQUEST_T * fw_io = (SCIF_SAS_IO_REQUEST_T *)object;

   SET_STATE_HANDLER(
      &fw_io->parent,
      scif_sas_io_request_state_handler_table,
      SCI_BASE_REQUEST_STATE_INITIAL
   );

   // Initial state is a transitional state to the constructed state
   sci_base_state_machine_change_state(
      &fw_io->parent.parent.state_machine, SCI_BASE_REQUEST_STATE_CONSTRUCTED
   );
}

/**
 * @brief This method implements the actions taken when entering the
 *        CONSTRUCTED state.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_IO_REQUEST object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_io_request_constructed_state_enter(
   SCI_BASE_OBJECT_T *object
)
{
   SCIF_SAS_IO_REQUEST_T * fw_io = (SCIF_SAS_IO_REQUEST_T *)object;

   SET_STATE_HANDLER(
      &fw_io->parent,
      scif_sas_io_request_state_handler_table,
      SCI_BASE_REQUEST_STATE_CONSTRUCTED
   );
}

/**
 * @brief This method implements the actions taken when entering the
 *        STARTED state.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_IO_REQUEST object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_io_request_started_state_enter(
   SCI_BASE_OBJECT_T *object
)
{
   SCIF_SAS_IO_REQUEST_T * fw_io = (SCIF_SAS_IO_REQUEST_T *)object;

   SET_STATE_HANDLER(
      &fw_io->parent,
      scif_sas_io_request_state_handler_table,
      SCI_BASE_REQUEST_STATE_STARTED
   );
}

/**
 * @brief This method implements the actions taken when entering the
 *        COMPLETED state.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_IO_REQUEST object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_io_request_completed_state_enter(
   SCI_BASE_OBJECT_T *object
)
{
   SCIF_SAS_IO_REQUEST_T * fw_io = (SCIF_SAS_IO_REQUEST_T *)object;

   SET_STATE_HANDLER(
      &fw_io->parent,
      scif_sas_io_request_state_handler_table,
      SCI_BASE_REQUEST_STATE_COMPLETED
   );
}

/**
 * @brief This method implements the actions taken when entering the
 *        ABORTING state.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_IO_REQUEST object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_io_request_aborting_state_enter(
   SCI_BASE_OBJECT_T *object
)
{
   SCIF_SAS_IO_REQUEST_T * fw_io = (SCIF_SAS_IO_REQUEST_T *)object;

   SCIF_LOG_WARNING((
      sci_base_object_get_logger(fw_io),
      SCIF_LOG_OBJECT_IO_REQUEST | SCIF_LOG_OBJECT_TASK_MANAGEMENT,
      "Domain:0x%x Device:0x%x IORequest:0x%x terminating\n",
      fw_io->parent.device->domain, fw_io->parent.device, fw_io
   ));

   SET_STATE_HANDLER(
      &fw_io->parent,
      scif_sas_io_request_state_handler_table,
      SCI_BASE_REQUEST_STATE_ABORTING
   );

   fw_io->parent.status = scif_sas_request_terminate_start(
                             &fw_io->parent, fw_io->parent.core_object
                          );
}

/**
 * @brief This method implements the actions taken when exiting the
 *        ABORTING state.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_IO_REQUEST object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_io_request_aborting_state_exit(
   SCI_BASE_OBJECT_T *object
)
{
   SCIF_SAS_REQUEST_T * fw_request = (SCIF_SAS_REQUEST_T *)object;
   scif_sas_request_terminate_complete(fw_request);
}

/**
 * @brief This method implements the actions taken when entering the
 *        FINAL state.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_IO_REQUEST object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_io_request_final_state_enter(
   SCI_BASE_OBJECT_T *object
)
{
   SCIF_SAS_IO_REQUEST_T * fw_io = (SCIF_SAS_IO_REQUEST_T *)object;

   SET_STATE_HANDLER(
      &fw_io->parent,
      scif_sas_io_request_state_handler_table,
      SCI_BASE_REQUEST_STATE_FINAL
   );
}

SCI_BASE_STATE_T scif_sas_io_request_state_table[SCI_BASE_REQUEST_MAX_STATES] =
{
   {
      SCI_BASE_REQUEST_STATE_INITIAL,
      scif_sas_io_request_initial_state_enter,
      NULL
   },
   {
      SCI_BASE_REQUEST_STATE_CONSTRUCTED,
      scif_sas_io_request_constructed_state_enter,
      NULL
   },
   {
      SCI_BASE_REQUEST_STATE_STARTED,
      scif_sas_io_request_started_state_enter,
      NULL
   },
   {
      SCI_BASE_REQUEST_STATE_COMPLETED,
      scif_sas_io_request_completed_state_enter,
      NULL
   },
   {
      SCI_BASE_REQUEST_STATE_ABORTING,
      scif_sas_io_request_aborting_state_enter,
      scif_sas_io_request_aborting_state_exit
   },
   {
      SCI_BASE_REQUEST_STATE_FINAL,
      scif_sas_io_request_final_state_enter,
      NULL
   },
};