
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
 * @brief This file contains core callback notificiations implemented by
 *        the framework for timers.
 */

#include <dev/isci/scil/sci_types.h>
#include <dev/isci/scil/scif_user_callback.h>
#include <dev/isci/scil/scic_user_callback.h>

#include <dev/isci/scil/scif_sas_controller.h>

void * scic_cb_timer_create(
   SCI_CONTROLLER_HANDLE_T   controller,
   SCI_TIMER_CALLBACK_T      timer_callback,
   void                    * cookie
)
{
   SCIF_SAS_CONTROLLER_T * fw_controller = (SCIF_SAS_CONTROLLER_T *)
                                         sci_object_get_association(controller);

   return scif_cb_timer_create(fw_controller, timer_callback, cookie);
}

// -----------------------------------------------------------------------------

void scic_cb_timer_destroy(
   SCI_CONTROLLER_HANDLE_T   controller,
   void                    * timer
)
{
   SCIF_SAS_CONTROLLER_T * fw_controller = (SCIF_SAS_CONTROLLER_T *)
                                         sci_object_get_association(controller);
   if (timer != NULL)
   {
      scif_cb_timer_destroy(fw_controller, timer);
      timer = NULL;
   }
}

// -----------------------------------------------------------------------------

void scic_cb_timer_start(
   SCI_CONTROLLER_HANDLE_T   controller,
   void                    * timer,
   U32                       milliseconds
)
{
   SCIF_SAS_CONTROLLER_T * fw_controller = (SCIF_SAS_CONTROLLER_T *)
                                         sci_object_get_association(controller);

   scif_cb_timer_start(fw_controller, timer, milliseconds);
}

// -----------------------------------------------------------------------------

void scic_cb_timer_stop(
   SCI_CONTROLLER_HANDLE_T   controller,
   void                    * timer
)
{
   SCIF_SAS_CONTROLLER_T * fw_controller = (SCIF_SAS_CONTROLLER_T *)
                                         sci_object_get_association(controller);

   scif_cb_timer_stop(fw_controller, timer);
}