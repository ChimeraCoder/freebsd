
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
 * @brief This file contains the mode page constants and data for the mode
 *        pages supported by this translation implementation.
 */

// DO NOT MOVE THIS INCLUDE STATEMENT! This include must occur before
// the below check for ENABLE_SATI_MODE_PAGES.
#include <dev/isci/scil/sati_types.h>

#if defined(ENABLE_SATI_MODE_PAGES)

#include <dev/isci/scil/sati_mode_pages.h>
#include <dev/isci/scil/intel_scsi.h>

//******************************************************************************
//* C O N S T A N T S
//******************************************************************************

#define SCSI_MODE_PAGE19_SAS_ID         0x6
#define SCSI_MODE_PAGE19_SUB1_PAGE_NUM  0x1
#define SCSI_MODE_PAGE19_SUB1_PC        0x59

//******************************************************************************
//* M O D E   P A G E S
//******************************************************************************

U8 sat_default_mode_page_01[] =
{
   SCSI_MODE_PAGE_READ_WRITE_ERROR, // Byte 0 - Page Code, SPF(0), PS(0)
   SCSI_MODE_PAGE_01_LENGTH-2,      // Byte 1 - Page Length
   0x80, // Byte 2 - AWRE, ARRE, TB, RC, EER, PER, DTE, DCR
   0x00, // Byte 3 - Read Retry Count

   0x00, // Byte 4 - Obsolete
   0x00, // Byte 5 - Obsolete
   0x00, // Byte 6 - Obsolete
   0x00, // Byte 7 - Restricted for MMC-4

   0x00, // Byte 8 - Write Retry Count
   0x00, // Byte 9 - Reserved
   0x00, // Byte 10 - Recovery Time Limit
   0x00, // Byte 11
};

U8 sat_changeable_mode_page_01[] =
{
   SCSI_MODE_PAGE_READ_WRITE_ERROR,
   SCSI_MODE_PAGE_01_LENGTH-2,
   0x00,
   0x00,

   0x00,
   0x00,
   0x00,
   0x00,

   0x00,
   0x00,
   0x00,
   0x00,
};

U8 sat_default_mode_page_02[] =
{
   SCSI_MODE_PAGE_DISCONNECT_RECONNECT, // Byte 0 - Page Code, SPF(0), PS(0)
   SCSI_MODE_PAGE_02_LENGTH-2,          // Byte 1 - Page Length
   0x00, // Byte 2 - Buffer Full Ratio
   0x00, // Byte 3 - Buffer Empty Ratio

   0x00, // Byte 4 - Bus Inactivity Limit
   0x00, // Byte 5
   0x00, // Byte 6 - Disconnect Time Limit
   0x00, // Byte 7

   0x00, // Byte 8 - Connect Time Limit
   0x00, // Byte 9
   0x00, // Byte 10 - Maximum Burst Size
   0x00, // Byte 11

   0x00, // Byte 12 - EMDP, FAIR_ARB, DIMM, DTDC
   0x00, // Byte 13
   0x00, // Byte 14 - First Burst Size
   0x00, // Byte 15
};

U8 sat_changeable_mode_page_02[] =
{
   SCSI_MODE_PAGE_DISCONNECT_RECONNECT,
   SCSI_MODE_PAGE_02_LENGTH-2,
   0x00,
   0x00,

   0x00,
   0x00,
   0x00,
   0x00,

   0x00,
   0x00,
   0x00,
   0x00,

   0x00,
   0x00,
   0x00,
   0x00,
};

U8 sat_default_mode_page_08[] =
{
   SCSI_MODE_PAGE_CACHING,     // Byte 0 - Page Code, SPF(0), PS(0)
   SCSI_MODE_PAGE_08_LENGTH-2, // Byte 1 - Page Length
   0x00, // Byte 2 - IC, ABPF, CAP, DISC, SIZE, WCE(1), MF, RCD
   0x00, // Byte 3 - Demand Read Retention Priority, Write Retention Priority

   0x00, // Byte 4 - Disable Pre-Fetch Transfer Length
   0x00, // Byte 5
   0x00, // Byte 6 - Minimum Pre-Fetch
   0x00, // Byte 7

   0x00, // Byte 8 - Maximum Pre-Fetch
   0x00, // Byte 9
   0x00, // Byte 10 - Maximum Pre-Fetch Ceiling
   0x00, // Byte 11

   0x00, // Byte 12 - FSW, LBCSS, DRA(0), Vendor Specific, NV_DIS
   0x00, // Byte 13 - Number of Cache Segments
   0x00, // Byte 14 - Cache Segment Size
   0x00, // Byte 15

   0x00, // Byte 16 - Reserved
   0x00, // Byte 17 - Non-Cache Segment Size
   0x00, // Byte 18
   0x00, // PAD
};

U8 sat_changeable_mode_page_08[] =
{
   SCSI_MODE_PAGE_CACHING,
   SCSI_MODE_PAGE_08_LENGTH-2,
   SCSI_MODE_PAGE_CACHE_PAGE_WCE_BIT,
   0x00,

   0x00,
   0x00,
   0x00,
   0x00,

   0x00,
   0x00,
   0x00,
   0x00,

   SCSI_MODE_PAGE_CACHE_PAGE_DRA_BIT,
   0x00,
   0x00,
   0x00,

   0x00,
   0x00,
   0x00,
   0x00, // PAD
};

U8 sat_default_mode_page_0A[] =
{
   SCSI_MODE_PAGE_CONTROL,     // Byte 0 - Page Code, SPF(0), PS(0)
   SCSI_MODE_PAGE_0A_LENGTH-2, // Byte 1 - Page Length
   0x00, // Byte 2 - TST(0), TMF_ONLY(0), D_SENSE(0), GLTSD(0), RLEC(0)
   0x10, // Byte 3 - Queue Algorithm(0), QErr(0)

   0x00, // Byte 4 - TAS(0), RAC(0), UA_(0), SWP(0)
   0x00, // Byte 5 - ATO(0), AUTOLOAD(0)
   0x00, // Byte 6
   0x00, // Byte 7

   0xFF, // Byte 8 - Unlimited Busy timeout
   0xFF, // Byte 9
   0x00, // Byte 10 - do not support self time compl time xlation
   0x00, // Byte 11
};

U8 sat_changeable_mode_page_0A[] =
{
   SCSI_MODE_PAGE_CONTROL,
   SCSI_MODE_PAGE_0A_LENGTH-2,
   0x00,
   0x00,

   0x00,
   0x00,
   0x00,
   0x00,

   0x00,
   0x00,
   0x00,
   0x00,
};

U8 sat_default_mode_page_19[] =
{
   SCSI_MODE_PAGE_PROTOCOL_SPECIFIC_PORT, // Byte 0 - PS, SPF, Page Code
   SCSI_MODE_PAGE_19_LENGTH-2,  // Byte 1 - Page Length
   SCSI_MODE_PAGE19_SAS_ID,     // Byte 2 - Rsvd, READY_LED,  ProtoID
   0x00, // PAD

   0xFF, // Byte 4 - IT NLT MSB, 0xFF retry forever
   0xFF, // Byte 5 - IT NLT LSB, 0xFF retry forever
   0x00, // Byte 6 - IRT MSB, 0x0 disable init resp timer
   0x00, // Byte 7 - IRT LSB, 0x0 disable init resp timer
};

U8 sat_changeable_mode_page_19[] =
{
   SCSI_MODE_PAGE_PROTOCOL_SPECIFIC_PORT,
   SCSI_MODE_PAGE_19_LENGTH-2,
   0x00,
   0x00,

   0x00,
   0x00,
   0x00,
   0x00,
};

U8 sat_default_mode_page_1C[] =
{
   SCSI_MODE_PAGE_INFORMATIONAL_EXCP_CONTROL, // Byte 0 - Page Code,
                                              // SPF(0), PS(0)
   SCSI_MODE_PAGE_1C_LENGTH-2,   // Byte 1 - Page Length
   SCSI_MODE_PAGE_DEXCPT_ENABLE, // Byte 2 - Perf, EBF, EWasc,
                                 // DExcpt(1), Test, LogErr
   0x06, // Byte 3 -- MRIE (6 == values only available upon request)

   0x00, // Byte 4 -- Interval Timer
   0x00, // Byte 5
   0x00, // Byte 6
   0x00, // Byte 7

   0x00, // Byte 8 -- Report Count
   0x00, // Byte 9
   0x00, // Byte 10
   0x00, // Byte 11
};

U8 sat_changeable_mode_page_1C[] =
{
   SCSI_MODE_PAGE_INFORMATIONAL_EXCP_CONTROL,
   SCSI_MODE_PAGE_1C_LENGTH-2,
   SCSI_MODE_PAGE_DEXCPT_ENABLE,
   0x00,

   0x00,
   0x00,
   0x00,
   0x00,

   0x00,
   0x00,
   0x00,
   0x00,
};

U8 sat_supported_mode_pages[] =
{
   SCSI_MODE_PAGE_READ_WRITE_ERROR,
   SCSI_MODE_PAGE_DISCONNECT_RECONNECT,
   SCSI_MODE_PAGE_CACHING,
   SCSI_MODE_PAGE_CONTROL,
   SCSI_MODE_PAGE_INFORMATIONAL_EXCP_CONTROL
};

U8 *sat_changeable_mode_pages[] =
{
   sat_changeable_mode_page_01,
   sat_changeable_mode_page_02,
   sat_changeable_mode_page_08,
   sat_changeable_mode_page_0A,
   sat_changeable_mode_page_1C
};

U8 *sat_default_mode_pages[] =
{
   sat_default_mode_page_01,
   sat_default_mode_page_02,
   sat_default_mode_page_08,
   sat_default_mode_page_0A,
   sat_default_mode_page_1C
};

U16 sat_mode_page_sizes[] =
{
   sizeof(sat_default_mode_page_01),
   sizeof(sat_default_mode_page_02),
   sizeof(sat_default_mode_page_08),
   sizeof(sat_default_mode_page_0A),
   sizeof(sat_default_mode_page_1C)
};

U16 sati_mode_page_get_page_index(
   U8  page_code
)
{
   U16 index;
   for (index = 0; index < SAT_SUPPORTED_MODE_PAGES_LENGTH; index++)
   {
      if (sat_supported_mode_pages[index] == page_code)
         return index;
   }

   return SATI_MODE_PAGE_UNSUPPORTED_INDEX;
}

#endif // defined(ENABLE_SATI_MODE_PAGES)