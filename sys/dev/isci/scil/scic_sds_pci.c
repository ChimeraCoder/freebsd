
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
 * @brief This file contains the method implementations utilized in writing
 *        out PCI data for the SCI core.
 */

#include <dev/isci/scil/scic_user_callback.h>

#include <dev/isci/scil/scic_sds_pci.h>
#include <dev/isci/scil/scic_sds_controller.h>

/**
 * @brief This method reads from the driver the BARs that are needed to
 *        determine the virtual memory space for the controller registers
 *
 * @param[in] this_controller The controller for which to read the base
 *            address registers.
 */
void scic_sds_pci_bar_initialization(
   SCIC_SDS_CONTROLLER_T* this_controller
)
{
#ifdef ARLINGTON_BUILD

   #define ARLINGTON_LEX_BAR  0
   #define ARLINGTON_SMU_BAR  1
   #define ARLINGTON_SCU_BAR  2
   #define LEX_REGISTER_OFFSET 0x40000

   this_controller->lex_registers =
      ((char *)scic_cb_pci_get_bar(
                     this_controller, ARLINGTON_LEX_BAR) + LEX_REGISTER_OFFSET);
   this_controller->smu_registers =
      (SMU_REGISTERS_T *)scic_cb_pci_get_bar(this_controller, ARLINGTON_SMU_BAR);
   this_controller->scu_registers =
      (SCU_REGISTERS_T *)scic_cb_pci_get_bar(this_controller, ARLINGTON_SCU_BAR);

#else // !ARLINGTON_BUILD

#if !defined(ENABLE_PCI_IO_SPACE_ACCESS)

   this_controller->smu_registers =
      (SMU_REGISTERS_T *)(
         (char *)scic_cb_pci_get_bar(this_controller, PATSBURG_SMU_BAR)
                +(0x4000 * this_controller->controller_index));
   this_controller->scu_registers =
      (SCU_REGISTERS_T *)(
         (char *)scic_cb_pci_get_bar(this_controller, PATSBURG_SCU_BAR)
                +(0x400000 * this_controller->controller_index));

#else // !defined(ENABLE_PCI_IO_SPACE_ACCESS)

   if (this_controller->controller_index == 0)
   {
      this_controller->smu_registers = (SMU_REGISTERS_T *)
         scic_cb_pci_get_bar(this_controller, PATSBURG_IO_SPACE_BAR0);
   }
   else
   {
      if (this_controller->pci_revision == SCU_PBG_HBA_REV_B0)
      {
         // SCU B0 violates PCI spec for size of IO bar this is corrected
         // in subsequent version of the hardware so we can safely use the
         // else condition below.
         this_controller->smu_registers = (SMU_REGISTERS_T *)
            (scic_cb_pci_get_bar(this_controller, PATSBURG_IO_SPACE_BAR0) + 0x100);
      }
      else
      {
         this_controller->smu_registers = (SMU_REGISTERS_T *)
            scic_cb_pci_get_bar(this_controller, PATSBURG_IO_SPACE_BAR1);
      }
   }

   // No need to get the bar.  We will be using the offset to write to
   // input/output ports via 0xA0 and 0xA4.
   this_controller->scu_registers = (SCU_REGISTERS_T *) 0;

#endif // !defined(ENABLE_PCI_IO_SPACE_ACCESS)

#endif // ARLINGTON_BUILD
}

#if defined(ENABLE_PCI_IO_SPACE_ACCESS) && !defined(ARLINGTON_BUILD)

/**
 * @brief This method will read from PCI memory for the SMU register
 *        space via IO space access.
 *
 * @param[in]  controller The controller for which to read a DWORD.
 * @param[in]  address This parameter depicts the address from
 *             which to read.
 *
 * @return The value being returned from the PCI memory location.
 *
 * @todo This PCI memory access calls likely need to be optimized into macro?
 */
U32 scic_sds_pci_read_smu_dword(
   SCI_CONTROLLER_HANDLE_T   controller,
   void                    * address
)
{
   return scic_cb_pci_read_dword(controller, address);
}

/**
 * @brief This method will write to PCI memory for the SMU register
 *        space via IO space access.
 *
 * @param[in]  controller The controller for which to read a DWORD.
 * @param[in]  address This parameter depicts the address into
 *             which to write.
 * @param[out] write_value This parameter depicts the value being written
 *             into the PCI memory location.
 *
 * @todo This PCI memory access calls likely need to be optimized into macro?
 */
void scic_sds_pci_write_smu_dword(
   SCI_CONTROLLER_HANDLE_T   controller,
   void                    * address,
   U32                       write_value
)
{
   scic_cb_pci_write_dword(controller, address, write_value);
}

/**
 * @brief This method will read from PCI memory for the SCU register
 *        space via IO space access.
 *
 * @param[in]  controller The controller for which to read a DWORD.
 * @param[in]  address This parameter depicts the address from
 *             which to read.
 *
 * @return The value being returned from the PCI memory location.
 *
 * @todo This PCI memory access calls likely need to be optimized into macro?
 */
U32 scic_sds_pci_read_scu_dword(
   SCI_CONTROLLER_HANDLE_T   controller,
   void                    * address
)
{
   SCIC_SDS_CONTROLLER_T * this_controller = (SCIC_SDS_CONTROLLER_T*)controller;

   scic_cb_pci_write_dword(
      controller,
      (void*) ((char *)(this_controller->smu_registers) + SCU_MMR_ADDRESS_WINDOW_OFFSET),
      (U32) address
   );

   return scic_cb_pci_read_dword(
             controller,
             (void*) ((char *)(this_controller->smu_registers) + SCU_MMR_DATA_WINDOW_OFFSET)
          );
}

/**
 * @brief This method will write to PCI memory for the SCU register
 *        space via IO space access.
 *
 * @param[in]  controller The controller for which to read a DWORD.
 * @param[in]  address This parameter depicts the address into
 *             which to write.
 * @param[out] write_value This parameter depicts the value being written
 *             into the PCI memory location.
 *
 * @todo This PCI memory access calls likely need to be optimized into macro?
 */
void scic_sds_pci_write_scu_dword(
   SCI_CONTROLLER_HANDLE_T   controller,
   void                    * address,
   U32                       write_value
)
{
   SCIC_SDS_CONTROLLER_T * this_controller = (SCIC_SDS_CONTROLLER_T*)controller;

   scic_cb_pci_write_dword(
      controller,
      (void*) ((char *)(this_controller->smu_registers) + SCU_MMR_ADDRESS_WINDOW_OFFSET),
      (U32) address
   );

   scic_cb_pci_write_dword(
      controller,
      (void*) ((char *)(this_controller->smu_registers) + SCU_MMR_DATA_WINDOW_OFFSET),
      write_value
   );
}

#endif // defined(ENABLE_PCI_IO_SPACE_ACCESS)