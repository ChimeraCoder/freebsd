
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
 * @brief This file contains the implementation of the public methods for a
 *        SCIC_SDS_LIBRARY object.
 */

#include <dev/isci/scil/scic_library.h>
#include <dev/isci/scil/scic_sds_library.h>
#include <dev/isci/scil/scic_sds_controller.h>
#include <dev/isci/scil/scic_sds_request.h>
#include <dev/isci/scil/scic_sds_remote_device.h>
#include <dev/isci/scil/intel_pci.h>
#include <dev/isci/scil/scic_sds_pci.h>
#include <dev/isci/scil/scu_constants.h>

struct SCIC_SDS_CONTROLLER;

#define SCIC_LIBRARY_CONTROLLER_MEMORY_START(library) \
   ((char *)(library) + sizeof(SCIC_SDS_LIBRARY_T))

// ---------------------------------------------------------------------------

U32 scic_library_get_object_size(
   U8 max_controller_count
)
{
   return   sizeof(SCIC_SDS_LIBRARY_T)
          + scic_sds_controller_get_object_size() * max_controller_count;
}

// ---------------------------------------------------------------------------

SCI_LIBRARY_HANDLE_T scic_library_construct(
   void                    * library_memory,
   U8                        max_controller_count
)
{
   SCI_STATUS status;
   SCIC_SDS_LIBRARY_T *this_library;

   this_library = (SCIC_SDS_LIBRARY_T *)library_memory;

   this_library->max_controller_count = max_controller_count;

   this_library->controllers =
      (SCIC_SDS_CONTROLLER_T *)((char *)library_memory + sizeof(SCIC_SDS_LIBRARY_T));

   SCI_BASE_LIBRARY_CONSTRUCT(this_library,
                              &this_library->parent,
                              max_controller_count,
                              struct SCIC_SDS_CONTROLLER,
                              status);
   return this_library;
}

// ---------------------------------------------------------------------------

void scic_library_set_pci_info(
   SCI_LIBRARY_HANDLE_T      library,
   SCI_PCI_COMMON_HEADER_T * pci_header
)
{
   SCIC_SDS_LIBRARY_T *this_library;
   this_library = (SCIC_SDS_LIBRARY_T *)library;

   this_library->pci_device   = pci_header->device_id;

#if defined(PBG_HBA_A0_BUILD)
   this_library->pci_revision = SCIC_SDS_PCI_REVISION_A0;
#elif defined(PBG_HBA_A2_BUILD)
   this_library->pci_revision = SCIC_SDS_PCI_REVISION_A2;
#elif defined(PBG_HBA_BETA_BUILD)
   this_library->pci_revision = SCIC_SDS_PCI_REVISION_B0;
#elif defined(PBG_BUILD)
   // The SCU PCI function revision ID for A0/A2 is not populated
   // properly.  As a result, we force the revision ID to A2 for
   // this situation.  Therefore, the standard PBG build will not
   // work for A0.
   if (pci_header->revision == SCIC_SDS_PCI_REVISION_A0)
      this_library->pci_revision = SCIC_SDS_PCI_REVISION_A2;
   else
      this_library->pci_revision = pci_header->revision;
#endif
}

// ---------------------------------------------------------------------------

SCI_STATUS scic_library_allocate_controller(
   SCI_LIBRARY_HANDLE_T    library,
   SCI_CONTROLLER_HANDLE_T *new_controller
)
{
   SCI_STATUS status;
   SCIC_SDS_LIBRARY_T *this_library;

   this_library = (SCIC_SDS_LIBRARY_T *)library;

   if (
         (  (this_library->pci_device >= 0x1D60)
         && (this_library->pci_device <= 0x1D62)
         )
      || (  (this_library->pci_device >= 0x1D64)
         && (this_library->pci_device <= 0x1D65)
         )
      || (  (this_library->pci_device >= 0x1D68)
         && (this_library->pci_device <= 0x1D6F)
         )
      )
   {
      SCI_BASE_LIBRARY_ALLOCATE_CONTROLLER(
         this_library, new_controller, &status);
   }
   else
      status = SCI_FAILURE_UNSUPPORTED_PCI_DEVICE_ID;

   return status;
}

// ---------------------------------------------------------------------------

SCI_STATUS scic_library_free_controller(
   SCI_LIBRARY_HANDLE_T library,
   SCI_CONTROLLER_HANDLE_T controller
)
{
   SCI_STATUS status;
   SCIC_SDS_LIBRARY_T *this_library;
   this_library = (SCIC_SDS_LIBRARY_T *)library;

   SCI_BASE_LIBRARY_FREE_CONTROLLER(
      this_library, controller, struct SCIC_SDS_CONTROLLER, &status);

   return status;
}

// ---------------------------------------------------------------------------

U8 scic_library_get_pci_device_controller_count(
   SCI_LIBRARY_HANDLE_T library
)
{
   SCIC_SDS_LIBRARY_T *this_library;
   U16 device_id;

   this_library = (SCIC_SDS_LIBRARY_T *)library;
   device_id = this_library->pci_device;

   //Check if we are on a b0 or c0 which has 2 controllers
   if (
         // Warning: If 0x1d66 is ever defined to be a single controller
         //          this logic will fail.
         //          If 0x1d63 or 0x1d67 is ever defined to be dual
         //          controller this logic will fail.
         ((device_id & 0xFFF1) == 0x1D60)
      && (
            (this_library->pci_revision == SCU_PBG_HBA_REV_B0)
         || (this_library->pci_revision == SCU_PBG_HBA_REV_C0)
         || (this_library->pci_revision == SCU_PBG_HBA_REV_C1)
         )
      )
      return 2;
   else
      return 1;
}

// ---------------------------------------------------------------------------

U32 scic_library_get_max_sge_size(
   SCI_LIBRARY_HANDLE_T library
)
{
   return SCU_IO_REQUEST_MAX_SGE_SIZE;
}

// ---------------------------------------------------------------------------

U32 scic_library_get_max_sge_count(
   SCI_LIBRARY_HANDLE_T library
)
{
   return SCU_IO_REQUEST_SGE_COUNT;
}

// ---------------------------------------------------------------------------

U32 scic_library_get_max_io_length(
   SCI_LIBRARY_HANDLE_T library
)
{
   return SCU_IO_REQUEST_MAX_TRANSFER_LENGTH;
}

// ---------------------------------------------------------------------------

U16 scic_library_get_min_timer_count(void)
{
   return (U16) (scic_sds_controller_get_min_timer_count()
               + scic_sds_remote_device_get_min_timer_count()
               + scic_sds_request_get_min_timer_count());
}

// ---------------------------------------------------------------------------

U16 scic_library_get_max_timer_count(void)
{
   return (U16) (scic_sds_controller_get_max_timer_count()
               + scic_sds_remote_device_get_max_timer_count()
               + scic_sds_request_get_max_timer_count());
}

/**
 *
 */
U8 scic_sds_library_get_controller_index(
   SCIC_SDS_LIBRARY_T    * library,
   SCIC_SDS_CONTROLLER_T * controller
)
{
   U8 index;

   for (index = 0; index < library->max_controller_count; index++)
   {
      if (controller == &library->controllers[index])
      {
         return index;
      }
   }

   return 0xff;
}