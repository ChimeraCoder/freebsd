
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
 * @brief This file provides the implementation for the observer role
 *        for a state machine.  A state machine observer is notified when
 *        the state machine changes states.
 */

#include <dev/isci/scil/sci_base_state_machine.h>
#include <dev/isci/scil/sci_base_state_machine_observer.h>

#if defined(SCI_LOGGING)

#define SCI_BASE_INVALID_SUBJECT_STATE 0xFFFF

//******************************************************************************
//* P R O T E C T E D    M E T H O D S
//******************************************************************************

void sci_base_state_machine_observer_default_update(
   SCI_BASE_OBSERVER_T *this_observer,
   SCI_BASE_SUBJECT_T  *the_subject
)
{
   SCI_BASE_STATE_MACHINE_OBSERVER_T *state_machine_observer;

   state_machine_observer = (SCI_BASE_STATE_MACHINE_OBSERVER_T *)this_observer;

   state_machine_observer->subject_state =
      sci_base_state_machine_get_state((SCI_BASE_STATE_MACHINE_T *)the_subject);
}

// ---------------------------------------------------------------------------

void sci_base_state_machine_observer_construct(
   SCI_BASE_STATE_MACHINE_OBSERVER_T *this_observer
)
{
   this_observer->parent.update = sci_base_state_machine_observer_default_update;

   this_observer->subject_state = SCI_BASE_INVALID_SUBJECT_STATE;
}

#endif // defined(SCI_LOGGING)