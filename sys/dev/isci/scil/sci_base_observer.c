
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
 * @brief This file implements the functionality common to all observer
 *        objects.
 */

#include <dev/isci/scil/sci_types.h>
#include <dev/isci/scil/sci_base_subject.h>
#include <dev/isci/scil/sci_base_observer.h>

#if defined(SCI_LOGGING)

//******************************************************************************
//* P U B L I C   M E T H O D S
//******************************************************************************

void sci_base_observer_construct(
   struct SCI_BASE_OBSERVER *this_observer,
   SCI_BASE_OBSERVER_UPDATE_T update
)
{
   this_observer->next = NULL;
   this_observer->update = update;
}

// ---------------------------------------------------------------------------

void sci_base_observer_initialize(
   SCI_BASE_OBSERVER_T        * the_observer,
   SCI_BASE_OBSERVER_UPDATE_T   update,
   SCI_BASE_SUBJECT_T         * the_subject
)
{
   sci_base_observer_construct(the_observer, update);
   sci_base_subject_attach_observer(the_subject, the_observer);
}

// ---------------------------------------------------------------------------

void sci_base_observer_update(
   SCI_BASE_OBSERVER_T *this_observer,
   SCI_BASE_SUBJECT_T  *the_subject
)
{
   if (this_observer->update != NULL)
   {
      this_observer->update(this_observer, the_subject);
   }
}

#endif // defined(SCI_LOGGING)