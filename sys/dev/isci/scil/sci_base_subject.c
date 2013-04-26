
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
 * @brief This file contains the base subject method implementations and
 *        any constants or structures private to the base subject object.
 *        A subject is a participant in the observer design pattern.  A
 *        subject represents the object being observed.
 */

#include <dev/isci/scil/sci_types.h>
#include <dev/isci/scil/sci_base_subject.h>
#include <dev/isci/scil/sci_base_observer.h>

#if defined(SCI_LOGGING)

//******************************************************************************
//* P R O T E C T E D    M E T H O D S
//******************************************************************************

void sci_base_subject_construct(
   SCI_BASE_SUBJECT_T *this_subject
)
{
   this_subject->observer_list = NULL;
}

// ---------------------------------------------------------------------------

void sci_base_subject_notify(
   SCI_BASE_SUBJECT_T *this_subject
)
{
   SCI_BASE_OBSERVER_T *this_observer = this_subject->observer_list;

   while (this_observer != NULL)
   {
      sci_base_observer_update(this_observer, this_subject);

      this_observer = this_observer->next;
   }
}

// ---------------------------------------------------------------------------

void sci_base_subject_attach_observer(
   SCI_BASE_SUBJECT_T   *this_subject,
   SCI_BASE_OBSERVER_T  *observer
)
{
   observer->next = this_subject->observer_list;

   this_subject->observer_list = observer;
}

// ---------------------------------------------------------------------------

void sci_base_subject_detach_observer(
   SCI_BASE_SUBJECT_T   *this_subject,
   SCI_BASE_OBSERVER_T  *observer
)
{
   SCI_BASE_OBSERVER_T *current_observer = this_subject->observer_list;
   SCI_BASE_OBSERVER_T *previous_observer = NULL;

   // Search list for the item to remove
   while (
              current_observer != NULL
           && current_observer != observer
         )
   {
      previous_observer = current_observer;
      current_observer = current_observer->next;
   }

   // Was this observer in the list?
   if (current_observer == observer)
   {
      if (previous_observer != NULL)
      {
         // Remove from middle or end of list
         previous_observer->next = observer->next;
      }
      else
      {
         // Remove from the front of the list
         this_subject->observer_list = observer->next;
      }

      // protect the list so people dont follow bad pointers
      observer->next = NULL;
   }
}

#endif // defined(SCI_LOGGING)