
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
 * @brief This file contains all of the method implementations for the
 *        SCI_BASE_OBJECT object.
 */

#include <dev/isci/scil/sci_status.h>
#include <dev/isci/scil/sci_types.h>
#include <dev/isci/scil/sci_base_object.h>

//******************************************************************************
//* P U B L I C   M E T H O D S
//******************************************************************************

#if defined(SCI_OBJECT_USE_ASSOCIATION_FUNCTIONS)
void * sci_object_get_association(
   SCI_OBJECT_HANDLE_T object
)
{
   return ((SCI_BASE_OBJECT_T *) object)->associated_object;
}
#endif

// ---------------------------------------------------------------------------

#if defined(SCI_OBJECT_USE_ASSOCIATION_FUNCTIONS)
SCI_STATUS sci_object_set_association(
   SCI_OBJECT_HANDLE_T   object,
   void                * associated_object
)
{
   ((SCI_BASE_OBJECT_T *)object)->associated_object = associated_object;
   return SCI_SUCCESS;
}
#endif

// ---------------------------------------------------------------------------

void sci_base_object_construct(
   SCI_BASE_OBJECT_T      * base_object,
   struct SCI_BASE_LOGGER * logger
)
{
#if defined(SCI_LOGGING)
   base_object->logger = logger;
#endif // defined(SCI_LOGGING)
   base_object->associated_object = NULL;
}

// ---------------------------------------------------------------------------

SCI_LOGGER_HANDLE_T sci_object_get_logger(
   SCI_OBJECT_HANDLE_T object
)
{
#if defined(SCI_LOGGING)
   return sci_base_object_get_logger(object);
#else // defined(SCI_LOGGING)
   return NULL;
#endif // defined(SCI_LOGGING)
}