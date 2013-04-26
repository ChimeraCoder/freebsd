
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
 * @brief This file contains the method implementations required to
 *        translate the SCSI mode sense 10-byte commands.
 */

#if !defined(DISABLE_SATI_MODE_SENSE)

#include <dev/isci/scil/sati_mode_sense.h>
#include <dev/isci/scil/sati_mode_sense_10.h>
#include <dev/isci/scil/sati_mode_pages.h>
#include <dev/isci/scil/sati_callbacks.h>
#include <dev/isci/scil/sati_util.h>
#include <dev/isci/scil/intel_scsi.h>
#include <dev/isci/scil/intel_ata.h>

//******************************************************************************
//* P R I V A T E   M E T H O D S
//******************************************************************************

/**
 * @brief This method builds the mode parameter header for a 10-byte SCSI
 *        mode sense data response.  The parameter header is 4 bytes in
 *        size.
 *        For more information on the parameters passed to this method,
 *        please reference sati_translate_command().
 *
 * @param[in] identify This parameter specifies the ATA remote device's
 *            received IDENTIFY DEVICE data.
 * @param[in] mode_data_length This parameter specifies the amount of data
 *            to be returned as part of this mode sense request.
 *
 * @return This method returns the number of bytes written into the
 *         mode sense data buffer.
 */
static
U32 sati_mode_sense_10_build_header(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * scsi_io,
   ATA_IDENTIFY_DEVICE_DATA_T * identify,
   U16                          mode_data_length
)
{
   U8 * cdb = sati_cb_get_cdb_address(scsi_io);

   // Fill in the length of the mode parameter data returned (do not include
   // the size of the mode data length field in the total).  Adjust the
   // mode data length to not include the mode data length fields itself
   // (i.e. subtract 2).
   mode_data_length -= 2;
   sati_set_data_byte(sequence, scsi_io, 0, (U8)(mode_data_length >> 8) & 0xFF);
   sati_set_data_byte(sequence, scsi_io, 1, (U8)(mode_data_length & 0xFF));

   // Medium Type is 0 for SBC devices
   sati_set_data_byte(sequence, scsi_io, 2, SCSI_MODE_HEADER_MEDIUM_TYPE_SBC);

   // Write Protect (WP), Rsvd, DPOFUA, Rsvd
   if (sequence->device->capabilities & SATI_DEVICE_CAP_DMA_FUA_ENABLE)
      sati_set_data_byte(sequence,scsi_io,3,SCSI_MODE_SENSE_HEADER_FUA_ENABLE);
   else
      sati_set_data_byte(sequence, scsi_io, 3, 0);

   // Set the reserved bytes to 0.  The LONGLBA field in byte 4 is overridden
   // later in this method if LLBAA is enabled.
   sati_set_data_byte(sequence, scsi_io, 4, 0);
   sati_set_data_byte(sequence, scsi_io, 5, 0);

   // The MSB for the block descriptor length is never used since the
   // largest block descriptor in this translator is 16-bytes in size.
   sati_set_data_byte(sequence, scsi_io, 6, 0);

   // Set the LSB block descriptor length if block descriptors are utilized.
   if (sati_get_cdb_byte(cdb, 1) & SCSI_MODE_SENSE_DBD_ENABLE)
      sati_set_data_byte(sequence, scsi_io, 7, 0);
   else
   {
      // If Long Logical Block Address are allowed, then the block descriptor
      // is larger (16 bytes total).
      if (sati_get_cdb_byte(cdb, 1) & SCSI_MODE_SENSE_LLBAA_ENABLE)
      {
         sati_set_data_byte(sequence, scsi_io, 4, 1);
         sati_set_data_byte(
            sequence, scsi_io, 7, SCSI_MODE_SENSE_LLBA_BLOCK_DESCRIPTOR_LENGTH
         );
      }
      else
      {
         sati_set_data_byte(
            sequence, scsi_io, 7, SCSI_MODE_SENSE_STD_BLOCK_DESCRIPTOR_LENGTH
         );
      }
   }

   return SCSI_MODE_SENSE_10_HEADER_LENGTH;
}

static
U32 sati_mode_sense_10_build_llba_block_descriptor(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * scsi_io,
   ATA_IDENTIFY_DEVICE_DATA_T * identify,
   U32                          offset
)
{
   U32  lba_low     = 0;
   U32  lba_high    = 0;
   U32  sector_size = 0;

   // Extract the sector information (sector size, logical blocks) from
   // the retrieved ATA identify device data.
   sati_ata_identify_device_get_sector_info(
      identify, &lba_low, &lba_high, &sector_size
   );

   // Fill in the 8-byte logical block area
   sati_set_data_byte(sequence, scsi_io, offset,   (U8)((lba_high>>24) & 0xFF));
   sati_set_data_byte(sequence, scsi_io, offset+1, (U8)((lba_high>>16) & 0xFF));
   sati_set_data_byte(sequence, scsi_io, offset+2, (U8)((lba_high>>8)  & 0xFF));
   sati_set_data_byte(sequence, scsi_io, offset+3, (U8)(lba_high & 0xFF));
   sati_set_data_byte(sequence, scsi_io, offset+4, (U8)((lba_low>>24) & 0xFF));
   sati_set_data_byte(sequence, scsi_io, offset+5, (U8)((lba_low>>16) & 0xFF));
   sati_set_data_byte(sequence, scsi_io, offset+6, (U8)((lba_low>>8)  & 0xFF));
   sati_set_data_byte(sequence, scsi_io, offset+7, (U8)(lba_low & 0xFF));

   // Clear the reserved fields.
   sati_set_data_byte(sequence, scsi_io, offset+8, 0);
   sati_set_data_byte(sequence, scsi_io, offset+9, 0);
   sati_set_data_byte(sequence, scsi_io, offset+10, 0);
   sati_set_data_byte(sequence, scsi_io, offset+11, 0);

   // Fill in the four byte Block Length field
   sati_set_data_byte(sequence,scsi_io, offset+12, (U8)((sector_size>>24) & 0xFF));
   sati_set_data_byte(sequence,scsi_io, offset+13, (U8)((sector_size>>16) & 0xFF));
   sati_set_data_byte(sequence,scsi_io, offset+14, (U8)((sector_size>>8)  & 0xFF));
   sati_set_data_byte(sequence,scsi_io, offset+15, (U8)(sector_size & 0xFF));

   return SCSI_MODE_SENSE_LLBA_BLOCK_DESCRIPTOR_LENGTH;
}

/**
 * @brief This method perform the data translation common to all SCSI MODE
 *        SENSE 10 byte commands.  This includes building the mode page
 *        header and block descriptor (if requested).
 *        For more information on the parameters passed to this method,
 *        please reference sati_translate_command().
 *
 * @param[in] identify This parameter specifies the remote device's IDENTIFY
 *            DEVICE data to be used during translation.
 * @param[in] transfer_length This parameter specifies the size of the
 *            mode page (including header & block descriptor).
 *
 * @return This method returns the number of bytes written into the user's
 *         mode page data buffer.
 */
static
U32 sati_mode_sense_10_translate_data(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   ATA_IDENTIFY_DEVICE_DATA_T * identify,
   void                       * scsi_io,
   U16                          transfer_length
)
{
   U8  * cdb = sati_cb_get_cdb_address(scsi_io);
   U32   offset;

   offset = sati_mode_sense_10_build_header(
               sequence, scsi_io, identify, transfer_length
            );

   // Determine if the caller disabled block descriptors (DBD).  If not,
   // then generate a block descriptor.
   if ((sati_get_cdb_byte(cdb, 1) & SCSI_MODE_SENSE_DBD_ENABLE) == 0)
   {
      // If the user requested the Long LBA format descriptor, then build
      // it
      if (sati_get_cdb_byte(cdb, 1) & SCSI_MODE_SENSE_LLBAA_ENABLE)
         offset += sati_mode_sense_10_build_llba_block_descriptor(
                      sequence, scsi_io, identify, offset
                   );
      else
         offset += sati_mode_sense_build_std_block_descriptor(
                      sequence, scsi_io, identify, offset
                   );
   }

   return offset;
}

//******************************************************************************
//* P R O T E C T E D   M E T H O D S
//******************************************************************************

/**
 * @brief This method will translate the SCSI mode sense 6 byte command
 *        into corresponding ATA commands.  If the command is well-formed,
 *        then the translation will result in an ATA IDENTIFY DEVICE
 *        command.
 *        For more information on the parameters passed to this method,
 *        please reference sati_translate_command().
 *
 * @return Indicate if the command translation succeeded.
 * @retval SCI_SUCCESS This is returned if the command translation was
 *         successful.
 * @retval SATI_FAILURE_CHECK_RESPONSE_DATA This value is returned if
 *         sense data has been created as a result of something specified
 *         in the CDB.
 */
SATI_STATUS sati_mode_sense_10_translate_command(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * scsi_io,
   void                       * ata_io
)
{
   U8 * cdb = sati_cb_get_cdb_address(scsi_io);

   sequence->command_specific_data.scratch = 0;

   // Set the data length based on the allocation length field in the CDB.
   sequence->allocation_length = (sati_get_cdb_byte(cdb, 7) << 8) |
                                 (sati_get_cdb_byte(cdb, 8));

   return sati_mode_sense_translate_command(sequence, scsi_io, ata_io, 10);
}

/**
 * @brief This method will perform data translation from the supplied ATA
 *        input data (i.e. an ATA IDENTIFY DEVICE block) into a CACHING
 *        mode page format.  The data will be written into the user's mode
 *        page data buffer.  This function operates specifically for MODE
 *        SENSE 10 commands.
 *        For more information on the parameters passed to this method,
 *        please reference sati_translate_data().
 *
 * @return none.
 */
void sati_mode_sense_10_caching_translate_data(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * ata_input_data,
   void                       * scsi_io
)
{
   ATA_IDENTIFY_DEVICE_DATA_T * identify = (ATA_IDENTIFY_DEVICE_DATA_T*)
                                           ata_input_data;
   U16   data_length = sati_mode_sense_calculate_page_header(scsi_io, 10)
                       + SCSI_MODE_PAGE_08_LENGTH;
   U32   page_offset = sati_mode_sense_10_translate_data(
                          sequence, identify, scsi_io, data_length
                       );

   sati_mode_sense_caching_translate_data(
      sequence, scsi_io, identify, page_offset
   );
}

/**
 * @brief This method will perform data translation from the supplied ATA
 *        input data (i.e. an ATA IDENTIFY DEVICE block) into a INFORMATIONAL
 *        EXCEPTIONS CONTROL mode page format.  The data will be written
 *        into the user's mode page data buffer.  This function operates
 *        specifically for MODE SENSE 10 commands.
 *        For more information on the parameters passed to this method,
 *        please reference sati_translate_data().
 *
 * @return none.
 */
void sati_mode_sense_10_informational_excp_control_translate_data(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * ata_input_data,
   void                       * scsi_io
)
{
   ATA_IDENTIFY_DEVICE_DATA_T * identify = (ATA_IDENTIFY_DEVICE_DATA_T*)
                                           ata_input_data;
   U16   data_length  = sati_mode_sense_calculate_page_header(scsi_io, 10)
                        + SCSI_MODE_PAGE_1C_LENGTH;
   U32   page_offset  = sati_mode_sense_10_translate_data(
                           sequence, identify, scsi_io, data_length
                        );

   sati_mode_sense_informational_excp_control_translate_data(
      sequence, scsi_io, identify, page_offset
   );
}

/**
* @brief This method will perform data translation from the supplied ATA
*        input data (i.e. an ATA IDENTIFY DEVICE block) into a Read Write Error
*         mode page format.  The data will be written
*        into the user's mode page data buffer.  This function operates
*        specifically for MODE SENSE 10 commands.
*        For more information on the parameters passed to this method,
*        please reference sati_translate_data().
*
* @return none.
*/
void sati_mode_sense_10_read_write_error_translate_data(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * ata_input_data,
   void                       * scsi_io
)
{
   ATA_IDENTIFY_DEVICE_DATA_T * identify = (ATA_IDENTIFY_DEVICE_DATA_T*)
      ata_input_data;

   U16   data_length  = sati_mode_sense_calculate_page_header(scsi_io, 10)
      + SCSI_MODE_PAGE_01_LENGTH;

   U32   page_offset  = sati_mode_sense_10_translate_data(
                           sequence, identify, scsi_io, data_length
                        );

   sati_mode_sense_read_write_error_translate_data(
      sequence, scsi_io, identify, page_offset
   );
}

/**
* @brief This method will perform data translation from the supplied ATA
*        input data (i.e. an ATA IDENTIFY DEVICE block) into a Disconnect
*        Reconnect mode page format.  The data will be written
*        into the user's mode page data buffer.  This function operates
*        specifically for MODE SENSE 10 commands.
*        For more information on the parameters passed to this method,
*        please reference sati_translate_data().
*
* @return none.
*/
void sati_mode_sense_10_disconnect_reconnect_translate_data(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * ata_input_data,
   void                       * scsi_io
)
{
   ATA_IDENTIFY_DEVICE_DATA_T * identify = (ATA_IDENTIFY_DEVICE_DATA_T*)
      ata_input_data;

   U8   data_length = (U8) sati_mode_sense_calculate_page_header(scsi_io, 10)
      + SCSI_MODE_PAGE_02_LENGTH;

   U32  page_offset = sati_mode_sense_10_translate_data(
                        sequence, identify, scsi_io, data_length
                      );

   sati_mode_sense_disconnect_reconnect_translate_data(
      sequence, scsi_io, identify, page_offset
   );
}

/**
* @brief This method will perform data translation from the supplied ATA
*        input data (i.e. an ATA IDENTIFY DEVICE block) into a Control
*         mode page format.  The data will be written
*        into the user's mode page data buffer.  This function operates
*        specifically for MODE SENSE 10 commands.
*        For more information on the parameters passed to this method,
*        please reference sati_translate_data().
*
* @return none.
*/
void sati_mode_sense_10_control_translate_data(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * ata_input_data,
   void                       * scsi_io
)
{
   ATA_IDENTIFY_DEVICE_DATA_T * identify = (ATA_IDENTIFY_DEVICE_DATA_T*)
      ata_input_data;

   U8   data_length = (U8) sati_mode_sense_calculate_page_header(scsi_io, 10)
      + SCSI_MODE_PAGE_0A_LENGTH;

   U32  page_offset = sati_mode_sense_10_translate_data(
                         sequence, identify, scsi_io, data_length
                      );

   sati_mode_sense_control_translate_data(
      sequence, scsi_io, identify, page_offset
   );
}

/**
* @brief This method will perform data translation from the supplied ATA
*        input data (i.e. an ATA IDENTIFY DEVICE block) into a Power
*        Condition mode page format.  The data will be written
*        into the user's mode page data buffer.  This function operates
*        specifically for MODE SENSE 10 commands.
*        For more information on the parameters passed to this method,
*        please reference sati_translate_data().
*
* @return none.
*/
void sati_mode_sense_10_power_condition_translate_data(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * ata_input_data,
   void                       * scsi_io
)
{
   ATA_IDENTIFY_DEVICE_DATA_T * identify = (ATA_IDENTIFY_DEVICE_DATA_T*)
      ata_input_data;

   U8 data_length;
   U32  page_offset;

   data_length = (U8) sati_mode_sense_calculate_page_header(scsi_io, 10)
      + SCSI_MODE_PAGE_1A_LENGTH;

   page_offset = sati_mode_sense_10_translate_data(
      sequence, identify, scsi_io, data_length
   );

   sati_mode_sense_power_condition_translate_data(
      sequence, scsi_io, identify, page_offset
   );
}


/**
 * @brief This method will perform data translation from the supplied ATA
 *        input data (i.e. an ATA IDENTIFY DEVICE block) into an ALL
 *        PAGES mode page format.  The ALL PAGES mode page is basically a
 *        conglomeration of all mode pages and sub-pages into a single
 *        page.  The data will be written into the user's mode page
 *        data buffer.  This function operates specifically for MODE
 *        SENSE 10 commands.
 *        For more information on the parameters passed to this method,
 *        please reference sati_translate_data().
 *
 * @return none.
 */
void sati_mode_sense_10_all_pages_translate_data(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * ata_input_data,
   void                       * scsi_io
)
{
   ATA_IDENTIFY_DEVICE_DATA_T * identify = (ATA_IDENTIFY_DEVICE_DATA_T*)
                                           ata_input_data;
   U8   data_length = (U8) sati_mode_sense_calculate_page_header(scsi_io, 10)
                           + SCSI_MODE_PAGE_01_LENGTH
                           + SCSI_MODE_PAGE_02_LENGTH
                           + SCSI_MODE_PAGE_08_LENGTH
                           + SCSI_MODE_PAGE_0A_LENGTH
                           + SCSI_MODE_PAGE_1C_LENGTH;
   U32  page_offset = sati_mode_sense_10_translate_data(
                         sequence, identify, scsi_io, data_length
                      );

   sati_mode_sense_all_pages_translate_data(
      sequence, scsi_io, identify, page_offset
   );
}

#endif // !defined(DISABLE_SATI_MODE_SENSE)