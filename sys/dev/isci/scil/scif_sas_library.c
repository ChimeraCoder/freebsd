
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
 *        SCIF_SAS_LIBRARY object.
 */


#include <dev/isci/scil/scic_library.h>
#include <dev/isci/scil/sci_pool.h>

#include <dev/isci/scil/scif_sas_library.h>
#include <dev/isci/scil/scif_sas_logger.h>
#include <dev/isci/scil/scif_sas_controller.h>


/**
 * This macro simply calculates the size of the framework library.  This
 * includes the memory for each controller object.
 */
#define SCIF_LIBRARY_SIZE(max_controllers)                             \
(                                                                      \
   sizeof(SCIF_SAS_LIBRARY_T) +                                        \
   (sizeof(SCIF_SAS_CONTROLLER_T) * (max_controllers))                 \
)


//******************************************************************************
//* P U B L I C   M E T H O D S
//******************************************************************************


U32 scif_library_get_object_size(
   U8 max_controller_count
)
{
   return ( SCIF_LIBRARY_SIZE(max_controller_count) +
            scic_library_get_object_size(max_controller_count) );
}

// ---------------------------------------------------------------------------

SCI_LIBRARY_HANDLE_T scif_library_construct(
   void * library_memory,
   U8     max_controller_count
)
{
   SCI_STATUS status;
   SCIF_SAS_LIBRARY_T * fw_library = (SCIF_SAS_LIBRARY_T *) library_memory;

   // Just clear out the memory of the structure to be safe.
   memset(fw_library, 0, scif_library_get_object_size(max_controller_count));

   // Invoke the parent object constructor.
   SCI_BASE_LIBRARY_CONSTRUCT(fw_library,
                              &fw_library->parent,
                              max_controller_count,
                              struct SCIF_SAS_CONTROLLER,
                              status);

   // The memory for the framework controller objects start immediately
   // after the library object.
   fw_library->controllers = (SCIF_SAS_CONTROLLER_T*)
                             ((U8*)library_memory + sizeof(SCIF_SAS_LIBRARY_T));

   // Construct the core library.
   fw_library->core_object = scic_library_construct(
                                (U8 *)library_memory +
                                SCIF_LIBRARY_SIZE(max_controller_count),
                                max_controller_count
                             );

   // Ensure construction completed successfully for the core.
   if (fw_library->core_object != SCI_INVALID_HANDLE)
   {
      // Set the association in the core library to this framework library.
      sci_object_set_association(
         (SCI_OBJECT_HANDLE_T) fw_library->core_object,
         (void *) fw_library
      );

      return fw_library;
   }

   return SCI_INVALID_HANDLE;
}

// ---------------------------------------------------------------------------

SCI_STATUS scif_library_allocate_controller(
   SCI_LIBRARY_HANDLE_T      library,
   SCI_CONTROLLER_HANDLE_T * new_controller
)
{
   SCI_STATUS  status;

   // Ensure the user supplied a valid library handle.
   if (library != SCI_INVALID_HANDLE)
   {
      SCIF_SAS_LIBRARY_T * fw_library = (SCIF_SAS_LIBRARY_T *) library;

      // Allocate the framework library.
      SCI_BASE_LIBRARY_ALLOCATE_CONTROLLER(fw_library, new_controller, &status);
      if (status == SCI_SUCCESS)
      {
         SCIF_SAS_CONTROLLER_T * fw_controller;

         // Allocate the core controller and save the handle in the framework
         // controller object.
         fw_controller = (SCIF_SAS_CONTROLLER_T*) *new_controller;

         // Just clear out the memory of the structure to be safe.
         memset(fw_controller, 0, sizeof(SCIF_SAS_CONTROLLER_T));

         status = scic_library_allocate_controller(
                     fw_library->core_object, &(fw_controller->core_object)
                  );

         // Free the framework controller if the core controller allocation
         // failed.
         if (status != SCI_SUCCESS)
            scif_library_free_controller(library, fw_controller);
      }

      if (status != SCI_SUCCESS)
      {
         SCIF_LOG_WARNING((
            sci_base_object_get_logger(fw_library),
            SCIF_LOG_OBJECT_LIBRARY,
            "Library:0x%x Status:0x%x controller allocation failed\n",
            fw_library, status
         ));
      }
   }
   else
      status = SCI_FAILURE_INVALID_PARAMETER_VALUE;

   return status;
}

// ---------------------------------------------------------------------------

SCI_STATUS scif_library_free_controller(
   SCI_LIBRARY_HANDLE_T     library,
   SCI_CONTROLLER_HANDLE_T  controller
)
{
   SCI_STATUS  status;

   if ( (library != SCI_INVALID_HANDLE) && (controller != SCI_INVALID_HANDLE) )
   {
      SCI_STATUS              core_status;
      SCIF_SAS_LIBRARY_T    * fw_library    = (SCIF_SAS_LIBRARY_T*) library;
      SCIF_SAS_CONTROLLER_T * fw_controller = (SCIF_SAS_CONTROLLER_T*) controller;

      core_status = scic_library_free_controller(
                       fw_library->core_object, fw_controller->core_object
                    );

      scif_sas_controller_destruct(fw_controller);

      SCI_BASE_LIBRARY_FREE_CONTROLLER(
         (SCIF_SAS_LIBRARY_T *) library,
         controller,
         SCIF_SAS_CONTROLLER_T,
         &status
      );

      if ( (status == SCI_SUCCESS) && (core_status != SCI_SUCCESS) )
         status = core_status;

      if (status != SCI_SUCCESS)
      {
         SCIF_LOG_WARNING((
            sci_base_object_get_logger(fw_library),
            SCIF_LOG_OBJECT_LIBRARY,
            "Library:0x%x Controller:0x%x Status:0x%x free controller failed\n",
            fw_library, fw_controller, status
         ));
      }
   }
   else
      status = SCI_FAILURE_INVALID_PARAMETER_VALUE;

   return status;
}

// ---------------------------------------------------------------------------

SCI_LIBRARY_HANDLE_T scif_library_get_scic_handle(
   SCI_LIBRARY_HANDLE_T   scif_library
)
{
   SCIF_SAS_LIBRARY_T * fw_library = (SCIF_SAS_LIBRARY_T*) scif_library;

   return fw_library->core_object;
}

// ---------------------------------------------------------------------------

#define SCIF_SAS_LIBRARY_MAX_TIMERS 32

U16 scif_library_get_max_timer_count(
   void
)
{
   /// @todo Need to calculate the exact maximum number of timers needed.
   return SCIF_SAS_LIBRARY_MAX_TIMERS + scic_library_get_max_timer_count();
}

//******************************************************************************
//* P R O T E C T E D   M E T H O D S
//******************************************************************************