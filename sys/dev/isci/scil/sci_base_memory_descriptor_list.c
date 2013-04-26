
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
 * @brief This file contains the base implementation for the memory
 *        descriptor list.  This is currently comprised of MDL iterator
 *        methods.
 */

#include <dev/isci/scil/sci_base_memory_descriptor_list.h>

//******************************************************************************
//* P U B L I C   M E T H O D S
//******************************************************************************

void sci_mdl_first_entry(
   SCI_MEMORY_DESCRIPTOR_LIST_HANDLE_T mdl
)
{
   SCI_BASE_MEMORY_DESCRIPTOR_LIST_T * base_mdl = (SCI_BASE_MEMORY_DESCRIPTOR_LIST_T*) mdl;

   base_mdl->next_index = 0;

   // If this MDL is managing another MDL, then recursively rewind that MDL
   // object as well.
   if (base_mdl->next_mdl != SCI_INVALID_HANDLE)
      sci_mdl_first_entry(base_mdl->next_mdl);
}

// ---------------------------------------------------------------------------

void sci_mdl_next_entry(
   SCI_MEMORY_DESCRIPTOR_LIST_HANDLE_T mdl
)
{
   SCI_BASE_MEMORY_DESCRIPTOR_LIST_T * base_mdl = (SCI_BASE_MEMORY_DESCRIPTOR_LIST_T*) mdl;

   // If there is at least one more entry left in the array, then change
   // the next pointer to it.
   if (base_mdl->next_index < base_mdl->length)
      base_mdl->next_index++;
   else if (base_mdl->next_index == base_mdl->length)
   {
      // This MDL has exhausted it's set of entries.  If this MDL is managing
      // another MDL, then start iterating through that MDL.
      if (base_mdl->next_mdl != SCI_INVALID_HANDLE)
         sci_mdl_next_entry(base_mdl->next_mdl);
   }
}

// ---------------------------------------------------------------------------

SCI_PHYSICAL_MEMORY_DESCRIPTOR_T * sci_mdl_get_current_entry(
   SCI_MEMORY_DESCRIPTOR_LIST_HANDLE_T mdl
)
{
   SCI_BASE_MEMORY_DESCRIPTOR_LIST_T * base_mdl = (SCI_BASE_MEMORY_DESCRIPTOR_LIST_T*) mdl;

   if (base_mdl->next_index < base_mdl->length)
      return & base_mdl->mde_array[base_mdl->next_index];
   else if (base_mdl->next_index == base_mdl->length)
   {
      // This MDL has exhausted it's set of entries.  If this MDL is managing
      // another MDL, then return it's current entry.
      if (base_mdl->next_mdl != SCI_INVALID_HANDLE)
         return sci_mdl_get_current_entry(base_mdl->next_mdl);
   }

   return NULL;
}

//******************************************************************************
//* P R O T E C T E D   M E T H O D S
//******************************************************************************

void sci_base_mdl_construct(
   SCI_BASE_MEMORY_DESCRIPTOR_LIST_T * mdl,
   SCI_PHYSICAL_MEMORY_DESCRIPTOR_T  * mde_array,
   U32                                 mde_array_length,
   SCI_MEMORY_DESCRIPTOR_LIST_HANDLE_T next_mdl
)
{
   mdl->length     = mde_array_length;
   mdl->mde_array  = mde_array;
   mdl->next_index = 0;
   mdl->next_mdl   = next_mdl;
}

// ---------------------------------------------------------------------------

BOOL sci_base_mde_is_valid(
   SCI_PHYSICAL_MEMORY_DESCRIPTOR_T * mde,
   U32                                alignment,
   U32                                size,
   U16                                attributes
)
{
   // Only need the lower 32 bits to ensure alignment is met.
   U32 physical_address = sci_cb_physical_address_lower(mde->physical_address);

   if (
         ((physical_address & (alignment - 1)) != 0)
      || (mde->constant_memory_alignment != alignment)
      || (mde->constant_memory_size != size)
      || (mde->virtual_address == NULL)
      || (mde->constant_memory_attributes != attributes)
      )
   {
      return FALSE;
   }

   return TRUE;
}