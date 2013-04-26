
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
 * @brief This file provides the public and protected implementations for the
 *        state machine logger.  The state machine logger will provide debug
 *        log information on a state machine for each state transition.
 */

#include <dev/isci/scil/sci_base_state_machine_logger.h>

#if defined(SCI_LOGGING)

//******************************************************************************
//* P R O T E C T E D    M E T H O D S
//******************************************************************************

/**
 * This is the function that is called when the state machine wants to notify
 * this observer that there has been a state change.
 *
 * @param[in] observer The state machine logger that is observing the state
 *       machine.
 * @param[in] subject The state machine that is being observed.
 */
static
void sci_base_state_machine_logger_update(
   SCI_BASE_OBSERVER_T *observer,
   SCI_BASE_SUBJECT_T  *subject
)
{
   SCI_BASE_STATE_MACHINE_LOGGER_T *this_observer;
   this_observer = (SCI_BASE_STATE_MACHINE_LOGGER_T *)observer;

   this_observer->log_function(
      sci_base_object_get_logger(this_observer->log_object),
      this_observer->log_mask,
      "%s 0x%08x %s has transitioned from %d to %d\n",
      this_observer->log_object_name,
      this_observer->log_object,
      this_observer->log_state_machine_name,
      this_observer->parent.subject_state,
      sci_base_state_machine_get_state((SCI_BASE_STATE_MACHINE_T *)subject)
   );

   sci_base_state_machine_observer_default_update(
      &this_observer->parent.parent, subject
   );
}

//******************************************************************************
//* P U B L I C   M E T H O D S
//******************************************************************************

/**
 * This function will construct the state machine logger and attach it to the
 * state machine that is to be observed.
 *
 * @param[in] this_observer This is the state machine logger object that is
 *       going to observe the subject state machine.
 * @param[in] the_object This is the object that contains the state machine
 *       being observed it is used to report the address of the object for
 *       which a state transition has occurred.
 * @param[in] the_log_function This is the logging function to be used when a
 *       state machine transition occurs.  Since this is a base object type it
 *       does not actually know if the logging function is for the core or
 *       framework.
 * @param[in] the_object_name This is the name of the object that contains the
 *       state machine being observed.
 * @param[in] the_state_machine_name This is the name that will be displayed
 *       in the log string for the state machine being observed.
 * @param[in] the_object_mask This is the log object mask used when calling
 *       the logging function.
 *
 * @return Nothing
 */
void sci_base_state_machine_logger_construct(
   SCI_BASE_STATE_MACHINE_LOGGER_T             * this_observer,
   SCI_BASE_OBJECT_T                           * the_object,
   SCI_BASE_STATE_MACHINE_LOGGER_LOG_HANDLER_T   the_log_function,
   char                                        * log_object_name,
   char                                        * log_state_machine_name,
   U32                                           log_object_mask
)
{
   sci_base_state_machine_observer_construct(&this_observer->parent);

   this_observer->log_object             = the_object;
   this_observer->log_function           = the_log_function;
   this_observer->log_object_name        = log_object_name;
   this_observer->log_state_machine_name = log_state_machine_name;
   this_observer->log_mask               = log_object_mask;

   this_observer->parent.parent.update = sci_base_state_machine_logger_update;
}

/**
 * This is a helper function that will construct the state machine logger and
 * attach it to the state machine that is to be observed.
 *
 * @param[in] this_observer This is the state machine logger object that is
 *       going to observe the subject state machine.
 * @param[in] the_state_machine This is the state machine that is under
 *       observation.
 * @param[in] the_object This is the object that contains the state machine
 *       being observed it is used to report the address of the object for
 *       which a state transition has occurred.
 * @param[in] the_log_function This is the logging function to be used when a
 *       state machine transition occurs.  Since this is a base object type it
 *       does not actually know if the logging function is for the core or
 *       framework.
 * @param[in] the_object_name This is the name of the object that contains the
 *       state machine being observed.
 * @param[in] the_state_machine_name This is the name that will be displayed
 *       in the log string for the state machine being observed.
 * @param[in] the_object_mask This is the log object mask used when calling
 *       the logging function.
 *
 * @return Nothing
 */
void sci_base_state_machine_logger_initialize(
   SCI_BASE_STATE_MACHINE_LOGGER_T             * this_observer,
   SCI_BASE_STATE_MACHINE_T                    * the_state_machine,
   SCI_BASE_OBJECT_T                           * the_object,
   SCI_BASE_STATE_MACHINE_LOGGER_LOG_HANDLER_T   the_log_function,
   char                                        * log_object_name,
   char                                        * log_state_machine_name,
   U32                                           log_object_mask
)
{
   sci_base_state_machine_logger_construct(
      this_observer, the_object,
      the_log_function, log_object_name, log_state_machine_name, log_object_mask
   );

   sci_base_subject_attach_observer(
      &the_state_machine->parent, &this_observer->parent.parent
   );
}

/**
 * This is a helper function that will detach this observer from the state
 * machine that is being observerd.
 *
 * @param[in] this_observer This is the observer to detach from the state
 *       machine.
 * @parame[in] the_state_machine This is the state machine that is no longer
 *       going to be observed.
 *
 * @return Nothing
 */
void sci_base_state_machine_logger_deinitialize(
   SCI_BASE_STATE_MACHINE_LOGGER_T * this_observer,
   SCI_BASE_STATE_MACHINE_T        * the_state_machine
)
{
   sci_base_subject_detach_observer(
      &the_state_machine->parent, &this_observer->parent.parent
   );
}

#endif // defined(SCI_LOGGING)