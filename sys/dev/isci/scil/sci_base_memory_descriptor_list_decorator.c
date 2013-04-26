
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
 *        descriptor list decorator.  The decorator adds additional
 *        functionality that may be used by SCI users to help
 *        offload MDE processing.
 */

#include <dev/isci/scil/sci_memory_descriptor_list_decorator.h>

U32 sci_mdl_decorator_get_memory_size(
   SCI_MEMORY_DESCRIPTOR_LIST_HANDLE_T mdl,
   U32                                 attributes
)
{
   U32                                size = 0;
   SCI_PHYSICAL_MEMORY_DESCRIPTOR_T * mde;

   sci_mdl_first_entry(mdl);
   mde = sci_mdl_get_current_entry(mdl);
   while (mde != NULL)
   {
      if (  (mde->constant_memory_attributes == attributes)
         || (attributes == 0) )
         size += (mde->constant_memory_size + mde->constant_memory_alignment);

      sci_mdl_next_entry(mdl);
      mde = sci_mdl_get_current_entry(mdl);
   }

   return size;
}

// ---------------------------------------------------------------------------

void sci_mdl_decorator_assign_memory(
   SCI_MEMORY_DESCRIPTOR_LIST_HANDLE_T mdl,
   U32                                 attributes,
   POINTER_UINT                        virtual_address,
   SCI_PHYSICAL_ADDRESS                sci_physical_address
)
{
   SCI_PHYSICAL_MEMORY_DESCRIPTOR_T * mde;
   U64  physical_address;

   physical_address
      =   ((U64) sci_cb_physical_address_lower(sci_physical_address))
        | (((U64) sci_cb_physical_address_upper(sci_physical_address)) << 32);

   sci_mdl_first_entry(mdl);
   mde = sci_mdl_get_current_entry(mdl);
   while (mde != NULL)
   {
      // As long as the memory attribute for this MDE is equivalent to
      // those supplied by the caller, then fill out the appropriate
      // MDE fields.
      if (  (mde->constant_memory_attributes == attributes)
         || (attributes == 0) )
      {
         // Ensure the virtual address alignment rules are met.
         if ((virtual_address % mde->constant_memory_alignment) != 0)
         {
            virtual_address
               += (mde->constant_memory_alignment -
                   (virtual_address % mde->constant_memory_alignment));
         }

         // Ensure the physical address alignment rules are met.
         if ((physical_address % mde->constant_memory_alignment) != 0)
         {
            physical_address
               += (mde->constant_memory_alignment -
                   (physical_address % mde->constant_memory_alignment));
         }

         // Update the MDE with properly aligned address values.
         mde->virtual_address  = (void *)virtual_address;
         sci_cb_make_physical_address(
            mde->physical_address,
            (U32) (physical_address >> 32),
            (U32) (physical_address & 0xFFFFFFFF)
         );

         virtual_address  += mde->constant_memory_size;
         physical_address += mde->constant_memory_size;
      }

      // Move on to the next MDE
      sci_mdl_next_entry(mdl);
      mde = sci_mdl_get_current_entry (mdl);
   }
}