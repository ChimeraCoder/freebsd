
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
 * @brief This file contains all of the entrance and exit methods for each
 *        of the controller states defined by the SCI_BASE_CONTROLLER state
 *        machine.
 */

#include <dev/isci/scil/scic_controller.h>

#include <dev/isci/scil/scif_sas_logger.h>
#include <dev/isci/scil/scif_sas_controller.h>

//******************************************************************************
//* P R O T E C T E D    M E T H O D S
//******************************************************************************

/**
 * @brief This method implements the actions taken when entering the
 *        INITIAL state.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_CONTROLLER object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_controller_initial_state_enter(
   SCI_BASE_OBJECT_T * object
)
{
   SCIF_SAS_CONTROLLER_T * fw_controller = (SCIF_SAS_CONTROLLER_T *)object;

   SET_STATE_HANDLER(
      fw_controller,
      scif_sas_controller_state_handler_table,
      SCI_BASE_CONTROLLER_STATE_INITIAL
   );
}

/**
 * @brief This method implements the actions taken when entering the
 *        RESET state.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_CONTROLLER object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_controller_reset_state_enter(
   SCI_BASE_OBJECT_T * object
)
{
   SCIF_SAS_CONTROLLER_T * fw_controller = (SCIF_SAS_CONTROLLER_T *)object;
   U8 index;
   U16 smp_phy_index;

   SET_STATE_HANDLER(
      fw_controller,
      scif_sas_controller_state_handler_table,
      SCI_BASE_CONTROLLER_STATE_RESET
   );

   scif_sas_high_priority_request_queue_construct(
      &fw_controller->hprq, sci_base_object_get_logger(fw_controller)
   );

   // Construct the abstract element pool. This pool will store the
   // references to the framework's remote devices objects.
   sci_abstract_element_pool_construct(
      &fw_controller->free_remote_device_pool,
      fw_controller->remote_device_pool_elements,
      SCI_MAX_REMOTE_DEVICES
   );

   // Construct the domain objects.
   for (index = 0; index < SCI_MAX_DOMAINS; index++)
   {
      scif_sas_domain_construct(
         &fw_controller->domains[index], index, fw_controller
      );
   }

   //Initialize SMP PHY MEMORY LIST.
   sci_fast_list_init(&fw_controller->smp_phy_memory_list);

   for (smp_phy_index = 0;
        smp_phy_index < SCIF_SAS_SMP_PHY_COUNT;
        smp_phy_index++)
   {
      sci_fast_list_element_init(
         &fw_controller->smp_phy_array[smp_phy_index],
         &(fw_controller->smp_phy_array[smp_phy_index].list_element)
      );

      //insert to owning device's smp phy list.
      sci_fast_list_insert_tail(
         (&(fw_controller->smp_phy_memory_list)),
         (&(fw_controller->smp_phy_array[smp_phy_index].list_element))
      );
   }

   scif_sas_controller_set_default_config_parameters(fw_controller);

   fw_controller->internal_request_entries =
      SCIF_SAS_MAX_INTERNAL_REQUEST_COUNT;

   //@Todo: may need to verify all timers are released. Including domain's
   //operation timer and all the Internal IO's timer.

   //take care of the lock.
   scif_cb_lock_disassociate(fw_controller, &fw_controller->hprq.lock);
}

/**
 * @brief This method implements the actions taken when entering the
 *        INITIALIZING state.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_CONTROLLER object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_controller_initializing_state_enter(
   SCI_BASE_OBJECT_T * object
)
{
   SCIF_SAS_CONTROLLER_T * fw_controller = (SCIF_SAS_CONTROLLER_T *)object;

   SET_STATE_HANDLER(
      fw_controller,
      scif_sas_controller_state_handler_table,
      SCI_BASE_CONTROLLER_STATE_INITIALIZING
   );
}

/**
 * @brief This method implements the actions taken when entering the
 *        INITIALIZED state.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_CONTROLLER object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_controller_initialized_state_enter(
   SCI_BASE_OBJECT_T * object
)
{
   SCIF_SAS_CONTROLLER_T * fw_controller = (SCIF_SAS_CONTROLLER_T *)object;

   SET_STATE_HANDLER(
      fw_controller,
      scif_sas_controller_state_handler_table,
      SCI_BASE_CONTROLLER_STATE_INITIALIZED
   );
}

/**
 * @brief This method implements the actions taken when entering the
 *        STARTING state.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_CONTROLLER object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_controller_starting_state_enter(
   SCI_BASE_OBJECT_T * object
)
{
   SCIF_SAS_CONTROLLER_T * fw_controller = (SCIF_SAS_CONTROLLER_T *)object;

   SET_STATE_HANDLER(
      fw_controller,
      scif_sas_controller_state_handler_table,
      SCI_BASE_CONTROLLER_STATE_STARTING
   );
}

/**
 * @brief This method implements the actions taken when entering the
 *        READY state.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_CONTROLLER object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_controller_ready_state_enter(
   SCI_BASE_OBJECT_T * object
)
{
   SCIF_SAS_CONTROLLER_T * fw_controller = (SCIF_SAS_CONTROLLER_T *)object;

   SET_STATE_HANDLER(
      fw_controller,
      scif_sas_controller_state_handler_table,
      SCI_BASE_CONTROLLER_STATE_READY
   );
}

/**
 * @brief This method implements the actions taken when entering the
 *        STOPPING state.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_CONTROLLER object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_controller_stopping_state_enter(
   SCI_BASE_OBJECT_T * object
)
{
   SCIF_SAS_CONTROLLER_T * fw_controller = (SCIF_SAS_CONTROLLER_T *)object;

   SET_STATE_HANDLER(
      fw_controller,
      scif_sas_controller_state_handler_table,
      SCI_BASE_CONTROLLER_STATE_STOPPING
   );
}

/**
 * @brief This method implements the actions taken when entering the
 *        STOPPED state.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_CONTROLLER object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_controller_stopped_state_enter(
   SCI_BASE_OBJECT_T * object
)
{
   SCIF_SAS_CONTROLLER_T * fw_controller = (SCIF_SAS_CONTROLLER_T *)object;

   SET_STATE_HANDLER(
      fw_controller,
      scif_sas_controller_state_handler_table,
      SCI_BASE_CONTROLLER_STATE_STOPPED
   );
}

/**
 * @brief This method implements the actions taken when entering the
 *        RESETTING state.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_CONTROLLER object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_controller_resetting_state_enter(
   SCI_BASE_OBJECT_T * object
)
{
   SCIF_SAS_CONTROLLER_T * fw_controller = (SCIF_SAS_CONTROLLER_T *)object;

   SET_STATE_HANDLER(
      fw_controller,
      scif_sas_controller_state_handler_table,
      SCI_BASE_CONTROLLER_STATE_RESETTING
   );

   // Attempt to reset the core controller.
   fw_controller->operation_status = scic_controller_reset(
                                        fw_controller->core_object
                                     );
   if (fw_controller->operation_status == SCI_SUCCESS)
   {
      // Reset the framework controller.
      sci_base_state_machine_change_state(
         &fw_controller->parent.state_machine,
         SCI_BASE_CONTROLLER_STATE_RESET
      );
   }
   else
   {
      SCIF_LOG_ERROR((
         sci_base_object_get_logger(fw_controller),
         SCIF_LOG_OBJECT_CONTROLLER,
         "Controller: unable to successfully reset controller.\n"
      ));

      sci_base_state_machine_change_state(
         &fw_controller->parent.state_machine,
         SCI_BASE_CONTROLLER_STATE_FAILED
      );
   }
}

/**
 * @brief This method implements the actions taken when entering the
 *        FAILED state.
 *
 * @param[in]  object This parameter specifies the base object for which
 *             the state transition is occurring.  This is cast into a
 *             SCIF_SAS_CONTROLLER object in the method implementation.
 *
 * @return none
 */
static
void scif_sas_controller_failed_state_enter(
   SCI_BASE_OBJECT_T * object
)
{
   SCIF_SAS_CONTROLLER_T * fw_controller = (SCIF_SAS_CONTROLLER_T *)object;

   SCIF_LOG_ERROR((
      sci_base_object_get_logger(fw_controller),
      SCIF_LOG_OBJECT_CONTROLLER,
      "Controller: entered FAILED state.\n"
   ));

   SET_STATE_HANDLER(
      fw_controller,
      scif_sas_controller_state_handler_table,
      SCI_BASE_CONTROLLER_STATE_FAILED
   );

   if (fw_controller->parent.error != SCI_CONTROLLER_FATAL_MEMORY_ERROR)
   {
       //clean timers to avoid timer leak.
       scif_sas_controller_release_resource(fw_controller);

       //notify user.
       scif_cb_controller_error(fw_controller, fw_controller->parent.error);
   }
}

SCI_BASE_STATE_T
scif_sas_controller_state_table[SCI_BASE_CONTROLLER_MAX_STATES] =
{
   {
      SCI_BASE_CONTROLLER_STATE_INITIAL,
      scif_sas_controller_initial_state_enter,
      NULL,
   },
   {
      SCI_BASE_CONTROLLER_STATE_RESET,
      scif_sas_controller_reset_state_enter,
      NULL,
   },
   {
      SCI_BASE_CONTROLLER_STATE_INITIALIZING,
      scif_sas_controller_initializing_state_enter,
      NULL,
   },
   {
      SCI_BASE_CONTROLLER_STATE_INITIALIZED,
      scif_sas_controller_initialized_state_enter,
      NULL,
   },
   {
      SCI_BASE_CONTROLLER_STATE_STARTING,
      scif_sas_controller_starting_state_enter,
      NULL,
   },
   {
      SCI_BASE_CONTROLLER_STATE_READY,
      scif_sas_controller_ready_state_enter,
      NULL,
   },
   {
      SCI_BASE_CONTROLLER_STATE_RESETTING,
      scif_sas_controller_resetting_state_enter,
      NULL,
   },
   {
      SCI_BASE_CONTROLLER_STATE_STOPPING,
      scif_sas_controller_stopping_state_enter,
      NULL,
   },
   {
      SCI_BASE_CONTROLLER_STATE_STOPPED,
      scif_sas_controller_stopped_state_enter,
      NULL,
   },
   {
      SCI_BASE_CONTROLLER_STATE_FAILED,
      scif_sas_controller_failed_state_enter,
      NULL,
   }
};