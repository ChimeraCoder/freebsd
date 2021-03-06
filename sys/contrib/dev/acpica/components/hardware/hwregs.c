
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
/*******************************************************************************
 *
 * Module Name: hwregs - Read/write access functions for the various ACPI
 *                       control and status registers.
 *
 ******************************************************************************/
/*
 * Copyright (C) 2000 - 2013, Intel Corp.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    substantially similar to the "NO WARRANTY" disclaimer below
 *    ("Disclaimer") and any redistribution must be conditioned upon
 *    including a substantially similar Disclaimer requirement for further
 *    binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 */

#define __HWREGS_C__

#include <contrib/dev/acpica/include/acpi.h>
#include <contrib/dev/acpica/include/accommon.h>
#include <contrib/dev/acpica/include/acevents.h>

#define _COMPONENT          ACPI_HARDWARE
        ACPI_MODULE_NAME    ("hwregs")


#if (!ACPI_REDUCED_HARDWARE)

/* Local Prototypes */

static ACPI_STATUS
AcpiHwReadMultiple (
    UINT32                  *Value,
    ACPI_GENERIC_ADDRESS    *RegisterA,
    ACPI_GENERIC_ADDRESS    *RegisterB);

static ACPI_STATUS
AcpiHwWriteMultiple (
    UINT32                  Value,
    ACPI_GENERIC_ADDRESS    *RegisterA,
    ACPI_GENERIC_ADDRESS    *RegisterB);

#endif /* !ACPI_REDUCED_HARDWARE */

/******************************************************************************
 *
 * FUNCTION:    AcpiHwValidateRegister
 *
 * PARAMETERS:  Reg                 - GAS register structure
 *              MaxBitWidth         - Max BitWidth supported (32 or 64)
 *              Address             - Pointer to where the gas->address
 *                                    is returned
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Validate the contents of a GAS register. Checks the GAS
 *              pointer, Address, SpaceId, BitWidth, and BitOffset.
 *
 ******************************************************************************/

ACPI_STATUS
AcpiHwValidateRegister (
    ACPI_GENERIC_ADDRESS    *Reg,
    UINT8                   MaxBitWidth,
    UINT64                  *Address)
{

    /* Must have a valid pointer to a GAS structure */

    if (!Reg)
    {
        return (AE_BAD_PARAMETER);
    }

    /*
     * Copy the target address. This handles possible alignment issues.
     * Address must not be null. A null address also indicates an optional
     * ACPI register that is not supported, so no error message.
     */
    ACPI_MOVE_64_TO_64 (Address, &Reg->Address);
    if (!(*Address))
    {
        return (AE_BAD_ADDRESS);
    }

    /* Validate the SpaceID */

    if ((Reg->SpaceId != ACPI_ADR_SPACE_SYSTEM_MEMORY) &&
        (Reg->SpaceId != ACPI_ADR_SPACE_SYSTEM_IO))
    {
        ACPI_ERROR ((AE_INFO,
            "Unsupported address space: 0x%X", Reg->SpaceId));
        return (AE_SUPPORT);
    }

    /* Validate the BitWidth */

    if ((Reg->BitWidth != 8) &&
        (Reg->BitWidth != 16) &&
        (Reg->BitWidth != 32) &&
        (Reg->BitWidth != MaxBitWidth))
    {
        ACPI_ERROR ((AE_INFO,
            "Unsupported register bit width: 0x%X", Reg->BitWidth));
        return (AE_SUPPORT);
    }

    /* Validate the BitOffset. Just a warning for now. */

    if (Reg->BitOffset != 0)
    {
        ACPI_WARNING ((AE_INFO,
            "Unsupported register bit offset: 0x%X", Reg->BitOffset));
    }

    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiHwRead
 *
 * PARAMETERS:  Value               - Where the value is returned
 *              Reg                 - GAS register structure
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Read from either memory or IO space. This is a 32-bit max
 *              version of AcpiRead, used internally since the overhead of
 *              64-bit values is not needed.
 *
 * LIMITATIONS: <These limitations also apply to AcpiHwWrite>
 *      BitWidth must be exactly 8, 16, or 32.
 *      SpaceID must be SystemMemory or SystemIO.
 *      BitOffset and AccessWidth are currently ignored, as there has
 *          not been a need to implement these.
 *
 ******************************************************************************/

ACPI_STATUS
AcpiHwRead (
    UINT32                  *Value,
    ACPI_GENERIC_ADDRESS    *Reg)
{
    UINT64                  Address;
    UINT64                  Value64;
    ACPI_STATUS             Status;


    ACPI_FUNCTION_NAME (HwRead);


    /* Validate contents of the GAS register */

    Status = AcpiHwValidateRegister (Reg, 32, &Address);
    if (ACPI_FAILURE (Status))
    {
        return (Status);
    }

    /* Initialize entire 32-bit return value to zero */

    *Value = 0;

    /*
     * Two address spaces supported: Memory or IO. PCI_Config is
     * not supported here because the GAS structure is insufficient
     */
    if (Reg->SpaceId == ACPI_ADR_SPACE_SYSTEM_MEMORY)
    {
        Status = AcpiOsReadMemory ((ACPI_PHYSICAL_ADDRESS)
                    Address, &Value64, Reg->BitWidth);

        *Value = (UINT32) Value64;
    }
    else /* ACPI_ADR_SPACE_SYSTEM_IO, validated earlier */
    {
        Status = AcpiHwReadPort ((ACPI_IO_ADDRESS)
                    Address, Value, Reg->BitWidth);
    }

    ACPI_DEBUG_PRINT ((ACPI_DB_IO,
        "Read:  %8.8X width %2d from %8.8X%8.8X (%s)\n",
        *Value, Reg->BitWidth, ACPI_FORMAT_UINT64 (Address),
        AcpiUtGetRegionName (Reg->SpaceId)));

    return (Status);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiHwWrite
 *
 * PARAMETERS:  Value               - Value to be written
 *              Reg                 - GAS register structure
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Write to either memory or IO space. This is a 32-bit max
 *              version of AcpiWrite, used internally since the overhead of
 *              64-bit values is not needed.
 *
 ******************************************************************************/

ACPI_STATUS
AcpiHwWrite (
    UINT32                  Value,
    ACPI_GENERIC_ADDRESS    *Reg)
{
    UINT64                  Address;
    ACPI_STATUS             Status;


    ACPI_FUNCTION_NAME (HwWrite);


    /* Validate contents of the GAS register */

    Status = AcpiHwValidateRegister (Reg, 32, &Address);
    if (ACPI_FAILURE (Status))
    {
        return (Status);
    }

    /*
     * Two address spaces supported: Memory or IO. PCI_Config is
     * not supported here because the GAS structure is insufficient
     */
    if (Reg->SpaceId == ACPI_ADR_SPACE_SYSTEM_MEMORY)
    {
        Status = AcpiOsWriteMemory ((ACPI_PHYSICAL_ADDRESS)
                    Address, (UINT64) Value, Reg->BitWidth);
    }
    else /* ACPI_ADR_SPACE_SYSTEM_IO, validated earlier */
    {
        Status = AcpiHwWritePort ((ACPI_IO_ADDRESS)
                    Address, Value, Reg->BitWidth);
    }

    ACPI_DEBUG_PRINT ((ACPI_DB_IO,
        "Wrote: %8.8X width %2d   to %8.8X%8.8X (%s)\n",
        Value, Reg->BitWidth, ACPI_FORMAT_UINT64 (Address),
        AcpiUtGetRegionName (Reg->SpaceId)));

    return (Status);
}


#if (!ACPI_REDUCED_HARDWARE)
/*******************************************************************************
 *
 * FUNCTION:    AcpiHwClearAcpiStatus
 *
 * PARAMETERS:  None
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Clears all fixed and general purpose status bits
 *
 ******************************************************************************/

ACPI_STATUS
AcpiHwClearAcpiStatus (
    void)
{
    ACPI_STATUS             Status;
    ACPI_CPU_FLAGS          LockFlags = 0;


    ACPI_FUNCTION_TRACE (HwClearAcpiStatus);


    ACPI_DEBUG_PRINT ((ACPI_DB_IO, "About to write %04X to %8.8X%8.8X\n",
        ACPI_BITMASK_ALL_FIXED_STATUS,
        ACPI_FORMAT_UINT64 (AcpiGbl_XPm1aStatus.Address)));

    LockFlags = AcpiOsAcquireLock (AcpiGbl_HardwareLock);

    /* Clear the fixed events in PM1 A/B */

    Status = AcpiHwRegisterWrite (ACPI_REGISTER_PM1_STATUS,
                ACPI_BITMASK_ALL_FIXED_STATUS);
    if (ACPI_FAILURE (Status))
    {
        goto UnlockAndExit;
    }

    /* Clear the GPE Bits in all GPE registers in all GPE blocks */

    Status = AcpiEvWalkGpeList (AcpiHwClearGpeBlock, NULL);

UnlockAndExit:
    AcpiOsReleaseLock (AcpiGbl_HardwareLock, LockFlags);
    return_ACPI_STATUS (Status);
}


/*******************************************************************************
 *
 * FUNCTION:    AcpiHwGetBitRegisterInfo
 *
 * PARAMETERS:  RegisterId          - Index of ACPI Register to access
 *
 * RETURN:      The bitmask to be used when accessing the register
 *
 * DESCRIPTION: Map RegisterId into a register bitmask.
 *
 ******************************************************************************/

ACPI_BIT_REGISTER_INFO *
AcpiHwGetBitRegisterInfo (
    UINT32                  RegisterId)
{
    ACPI_FUNCTION_ENTRY ();


    if (RegisterId > ACPI_BITREG_MAX)
    {
        ACPI_ERROR ((AE_INFO, "Invalid BitRegister ID: 0x%X", RegisterId));
        return (NULL);
    }

    return (&AcpiGbl_BitRegisterInfo[RegisterId]);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiHwWritePm1Control
 *
 * PARAMETERS:  Pm1aControl         - Value to be written to PM1A control
 *              Pm1bControl         - Value to be written to PM1B control
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Write the PM1 A/B control registers. These registers are
 *              different than than the PM1 A/B status and enable registers
 *              in that different values can be written to the A/B registers.
 *              Most notably, the SLP_TYP bits can be different, as per the
 *              values returned from the _Sx predefined methods.
 *
 ******************************************************************************/

ACPI_STATUS
AcpiHwWritePm1Control (
    UINT32                  Pm1aControl,
    UINT32                  Pm1bControl)
{
    ACPI_STATUS             Status;


    ACPI_FUNCTION_TRACE (HwWritePm1Control);


    Status = AcpiHwWrite (Pm1aControl, &AcpiGbl_FADT.XPm1aControlBlock);
    if (ACPI_FAILURE (Status))
    {
        return_ACPI_STATUS (Status);
    }

    if (AcpiGbl_FADT.XPm1bControlBlock.Address)
    {
        Status = AcpiHwWrite (Pm1bControl, &AcpiGbl_FADT.XPm1bControlBlock);
    }
    return_ACPI_STATUS (Status);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiHwRegisterRead
 *
 * PARAMETERS:  RegisterId          - ACPI Register ID
 *              ReturnValue         - Where the register value is returned
 *
 * RETURN:      Status and the value read.
 *
 * DESCRIPTION: Read from the specified ACPI register
 *
 ******************************************************************************/

ACPI_STATUS
AcpiHwRegisterRead (
    UINT32                  RegisterId,
    UINT32                  *ReturnValue)
{
    UINT32                  Value = 0;
    ACPI_STATUS             Status;


    ACPI_FUNCTION_TRACE (HwRegisterRead);


    switch (RegisterId)
    {
    case ACPI_REGISTER_PM1_STATUS:           /* PM1 A/B: 16-bit access each */

        Status = AcpiHwReadMultiple (&Value,
                    &AcpiGbl_XPm1aStatus,
                    &AcpiGbl_XPm1bStatus);
        break;


    case ACPI_REGISTER_PM1_ENABLE:           /* PM1 A/B: 16-bit access each */

        Status = AcpiHwReadMultiple (&Value,
                    &AcpiGbl_XPm1aEnable,
                    &AcpiGbl_XPm1bEnable);
        break;


    case ACPI_REGISTER_PM1_CONTROL:          /* PM1 A/B: 16-bit access each */

        Status = AcpiHwReadMultiple (&Value,
                    &AcpiGbl_FADT.XPm1aControlBlock,
                    &AcpiGbl_FADT.XPm1bControlBlock);

        /*
         * Zero the write-only bits. From the ACPI specification, "Hardware
         * Write-Only Bits": "Upon reads to registers with write-only bits,
         * software masks out all write-only bits."
         */
        Value &= ~ACPI_PM1_CONTROL_WRITEONLY_BITS;
        break;


    case ACPI_REGISTER_PM2_CONTROL:          /* 8-bit access */

        Status = AcpiHwRead (&Value, &AcpiGbl_FADT.XPm2ControlBlock);
        break;


    case ACPI_REGISTER_PM_TIMER:             /* 32-bit access */

        Status = AcpiHwRead (&Value, &AcpiGbl_FADT.XPmTimerBlock);
        break;


    case ACPI_REGISTER_SMI_COMMAND_BLOCK:    /* 8-bit access */

        Status = AcpiHwReadPort (AcpiGbl_FADT.SmiCommand, &Value, 8);
        break;


    default:
        ACPI_ERROR ((AE_INFO, "Unknown Register ID: 0x%X",
            RegisterId));
        Status = AE_BAD_PARAMETER;
        break;
    }

    if (ACPI_SUCCESS (Status))
    {
        *ReturnValue = Value;
    }

    return_ACPI_STATUS (Status);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiHwRegisterWrite
 *
 * PARAMETERS:  RegisterId          - ACPI Register ID
 *              Value               - The value to write
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Write to the specified ACPI register
 *
 * NOTE: In accordance with the ACPI specification, this function automatically
 * preserves the value of the following bits, meaning that these bits cannot be
 * changed via this interface:
 *
 * PM1_CONTROL[0] = SCI_EN
 * PM1_CONTROL[9]
 * PM1_STATUS[11]
 *
 * ACPI References:
 * 1) Hardware Ignored Bits: When software writes to a register with ignored
 *      bit fields, it preserves the ignored bit fields
 * 2) SCI_EN: OSPM always preserves this bit position
 *
 ******************************************************************************/

ACPI_STATUS
AcpiHwRegisterWrite (
    UINT32                  RegisterId,
    UINT32                  Value)
{
    ACPI_STATUS             Status;
    UINT32                  ReadValue;


    ACPI_FUNCTION_TRACE (HwRegisterWrite);


    switch (RegisterId)
    {
    case ACPI_REGISTER_PM1_STATUS:           /* PM1 A/B: 16-bit access each */
        /*
         * Handle the "ignored" bit in PM1 Status. According to the ACPI
         * specification, ignored bits are to be preserved when writing.
         * Normally, this would mean a read/modify/write sequence. However,
         * preserving a bit in the status register is different. Writing a
         * one clears the status, and writing a zero preserves the status.
         * Therefore, we must always write zero to the ignored bit.
         *
         * This behavior is clarified in the ACPI 4.0 specification.
         */
        Value &= ~ACPI_PM1_STATUS_PRESERVED_BITS;

        Status = AcpiHwWriteMultiple (Value,
                    &AcpiGbl_XPm1aStatus,
                    &AcpiGbl_XPm1bStatus);
        break;


    case ACPI_REGISTER_PM1_ENABLE:           /* PM1 A/B: 16-bit access each */

        Status = AcpiHwWriteMultiple (Value,
                    &AcpiGbl_XPm1aEnable,
                    &AcpiGbl_XPm1bEnable);
        break;


    case ACPI_REGISTER_PM1_CONTROL:          /* PM1 A/B: 16-bit access each */

        /*
         * Perform a read first to preserve certain bits (per ACPI spec)
         * Note: This includes SCI_EN, we never want to change this bit
         */
        Status = AcpiHwReadMultiple (&ReadValue,
                    &AcpiGbl_FADT.XPm1aControlBlock,
                    &AcpiGbl_FADT.XPm1bControlBlock);
        if (ACPI_FAILURE (Status))
        {
            goto Exit;
        }

        /* Insert the bits to be preserved */

        ACPI_INSERT_BITS (Value, ACPI_PM1_CONTROL_PRESERVED_BITS, ReadValue);

        /* Now we can write the data */

        Status = AcpiHwWriteMultiple (Value,
                    &AcpiGbl_FADT.XPm1aControlBlock,
                    &AcpiGbl_FADT.XPm1bControlBlock);
        break;


    case ACPI_REGISTER_PM2_CONTROL:          /* 8-bit access */

        /*
         * For control registers, all reserved bits must be preserved,
         * as per the ACPI spec.
         */
        Status = AcpiHwRead (&ReadValue, &AcpiGbl_FADT.XPm2ControlBlock);
        if (ACPI_FAILURE (Status))
        {
            goto Exit;
        }

        /* Insert the bits to be preserved */

        ACPI_INSERT_BITS (Value, ACPI_PM2_CONTROL_PRESERVED_BITS, ReadValue);

        Status = AcpiHwWrite (Value, &AcpiGbl_FADT.XPm2ControlBlock);
        break;


    case ACPI_REGISTER_PM_TIMER:             /* 32-bit access */

        Status = AcpiHwWrite (Value, &AcpiGbl_FADT.XPmTimerBlock);
        break;


    case ACPI_REGISTER_SMI_COMMAND_BLOCK:    /* 8-bit access */

        /* SMI_CMD is currently always in IO space */

        Status = AcpiHwWritePort (AcpiGbl_FADT.SmiCommand, Value, 8);
        break;


    default:
        ACPI_ERROR ((AE_INFO, "Unknown Register ID: 0x%X",
            RegisterId));
        Status = AE_BAD_PARAMETER;
        break;
    }

Exit:
    return_ACPI_STATUS (Status);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiHwReadMultiple
 *
 * PARAMETERS:  Value               - Where the register value is returned
 *              RegisterA           - First ACPI register (required)
 *              RegisterB           - Second ACPI register (optional)
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Read from the specified two-part ACPI register (such as PM1 A/B)
 *
 ******************************************************************************/

static ACPI_STATUS
AcpiHwReadMultiple (
    UINT32                  *Value,
    ACPI_GENERIC_ADDRESS    *RegisterA,
    ACPI_GENERIC_ADDRESS    *RegisterB)
{
    UINT32                  ValueA = 0;
    UINT32                  ValueB = 0;
    ACPI_STATUS             Status;


    /* The first register is always required */

    Status = AcpiHwRead (&ValueA, RegisterA);
    if (ACPI_FAILURE (Status))
    {
        return (Status);
    }

    /* Second register is optional */

    if (RegisterB->Address)
    {
        Status = AcpiHwRead (&ValueB, RegisterB);
        if (ACPI_FAILURE (Status))
        {
            return (Status);
        }
    }

    /*
     * OR the two return values together. No shifting or masking is necessary,
     * because of how the PM1 registers are defined in the ACPI specification:
     *
     * "Although the bits can be split between the two register blocks (each
     * register block has a unique pointer within the FADT), the bit positions
     * are maintained. The register block with unimplemented bits (that is,
     * those implemented in the other register block) always returns zeros,
     * and writes have no side effects"
     */
    *Value = (ValueA | ValueB);
    return (AE_OK);
}


/******************************************************************************
 *
 * FUNCTION:    AcpiHwWriteMultiple
 *
 * PARAMETERS:  Value               - The value to write
 *              RegisterA           - First ACPI register (required)
 *              RegisterB           - Second ACPI register (optional)
 *
 * RETURN:      Status
 *
 * DESCRIPTION: Write to the specified two-part ACPI register (such as PM1 A/B)
 *
 ******************************************************************************/

static ACPI_STATUS
AcpiHwWriteMultiple (
    UINT32                  Value,
    ACPI_GENERIC_ADDRESS    *RegisterA,
    ACPI_GENERIC_ADDRESS    *RegisterB)
{
    ACPI_STATUS             Status;


    /* The first register is always required */

    Status = AcpiHwWrite (Value, RegisterA);
    if (ACPI_FAILURE (Status))
    {
        return (Status);
    }

    /*
     * Second register is optional
     *
     * No bit shifting or clearing is necessary, because of how the PM1
     * registers are defined in the ACPI specification:
     *
     * "Although the bits can be split between the two register blocks (each
     * register block has a unique pointer within the FADT), the bit positions
     * are maintained. The register block with unimplemented bits (that is,
     * those implemented in the other register block) always returns zeros,
     * and writes have no side effects"
     */
    if (RegisterB->Address)
    {
        Status = AcpiHwWrite (Value, RegisterB);
    }

    return (Status);
}

#endif /* !ACPI_REDUCED_HARDWARE */