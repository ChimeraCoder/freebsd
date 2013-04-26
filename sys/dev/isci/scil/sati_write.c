
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
 *        translate the SCSI write (6, 10, 12, or 16-byte) commands.
 */

#include <dev/isci/scil/sati_move.h>
#include <dev/isci/scil/sati_write.h>
#include <dev/isci/scil/sati_callbacks.h>
#include <dev/isci/scil/sati_util.h>

#include <dev/isci/scil/intel_ata.h>
#include <dev/isci/scil/intel_scsi.h>

//******************************************************************************
//* P R I V A T E   M E T H O D S
//******************************************************************************

/**
 * @brief This method performs the common translation functionality for
 *        all SCSI write operations that are 10 bytes in size or larger.
 *        Translated/Written items include:
 *        - Force Unit Access (FUA)
 *        - Sector Count/Transfer Length
 *        - Command register
 *
 * @param[in] sector_count This parameter specifies the number of sectors
 *            to be transferred by this request.
 * @param[in] device_head This parameter points to device head register
 *            that is to be written into the ATA task file (register FIS).
 *
 * @return Indicate if the command translation succeeded.
 * @see sati_move_set_sector_count() for additional return values.
 */
static
SATI_STATUS sati_write_large_translate_command(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * scsi_io,
   void                       * ata_io,
   U32                          sector_count,
   U8                         * device_head
)
{
   sequence->data_direction = SATI_DATA_DIRECTION_OUT;

   return sati_move_large_translate_command(
             sequence,
             scsi_io,
             ata_io,
             sector_count,
             device_head
          );
}

/**
 * @brief This method performs the common translation functionality for
 *        all SCSI write operations containing a 32-bit logical block
 *        address.
 *        Translated/Written items include:
 *        - Logical Block Address (LBA)
 *        - Force Unit Access (FUA)
 *        - Sector Count/Transfer Length
 *        - Command register
 *        For more information on the parameters passed to this method,
 *        please reference sati_translate_command().
 *
 * @param[in] sector_count This parameter specifies the number of sectors
 *            to be transferred by this request.
 * @param[in] control_byte_offset This parameter specifies the byte offset
 *            into the command descriptor block at which the control byte
 *            is located.
 *
 * @return Indicate if the command translation succeeded.
 * @see sati_move_32_bit_lba_translate_command(), sati_move_set_sector_count()
 *      for additional return values.
 */
static
SATI_STATUS sati_write_32_bit_lba_translate_command(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * scsi_io,
   void                       * ata_io,
   U32                          sector_count,
   U8                           control_byte_offset
)
{
   U8           device_head = 0;
   SATI_STATUS  status;

   status = sati_write_large_translate_command(
               sequence, scsi_io, ata_io, sector_count, &device_head
            );

   if (status == SATI_SUCCESS)
   {
      status = sati_move_32_bit_lba_translate_command(
                  sequence, scsi_io, ata_io, device_head
               );
   }

   return status;
}

//******************************************************************************
//* P U B L I C   M E T H O D S
//******************************************************************************

/**
 * @brief This method will translate the SCSI write command into a
 *        corresponding ATA write 6 command.  Depending upon the capabilities
 *        supported by the target different ATA commands can be selected.
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
SATI_STATUS sati_write_6_translate_command(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * scsi_io,
   void                       * ata_io
)
{
   if(sati_device_state_stopped(sequence, scsi_io))
   {
      return SATI_FAILURE_CHECK_RESPONSE_DATA;
   }
   else
   {
      sequence->data_direction = SATI_DATA_DIRECTION_OUT;
      sequence->type           = SATI_SEQUENCE_WRITE_6;

      return sati_move_small_translate_command(sequence, scsi_io, ata_io);
   }
}

/**
 * @brief This method will translate the SCSI write 10 command into a
 *        corresponding ATA write command.  Depending upon the capabilities
 *        supported by the target different ATA commands can be selected.
 *        It ensures that all translation required for this command is
 *        performed successfully.
 *        For more information on the parameters passed to this method,
 *        please reference sati_translate_command().
 *
 * @return Indicate if the command translation succeeded.
 * @see sati_write_32_bit_lba_translate_command() for return values.
 */
SATI_STATUS sati_write_10_translate_command(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * scsi_io,
   void                       * ata_io
)
{
   U8  * cdb          = sati_cb_get_cdb_address(scsi_io);
   U32   sector_count = (sati_get_cdb_byte(cdb, 7) << 8) |
                        (sati_get_cdb_byte(cdb, 8));

   if(sati_device_state_stopped(sequence, scsi_io))
   {
      return SATI_FAILURE_CHECK_RESPONSE_DATA;
   }
   else
   {
      sequence->type     = SATI_SEQUENCE_WRITE_10;

      return sati_write_32_bit_lba_translate_command(
                sequence, scsi_io, ata_io, sector_count, 9
             );
   }
}

/**
 * @brief This method will translate the SCSI write 12 command into a
 *        corresponding ATA write command.  Depending upon the capabilities
 *        supported by the target different ATA commands can be selected.
 *        It ensures that all translation required for this command is
 *        performed successfully.
 *        For more information on the parameters passed to this method,
 *        please reference sati_translate_command().
 *
 * @return Indicate if the command translation succeeded.
 * @see sati_write_32_bit_lba_translate_command() for return values.
 */
SATI_STATUS sati_write_12_translate_command(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * scsi_io,
   void                       * ata_io
)
{
   U8  * cdb          = sati_cb_get_cdb_address(scsi_io);
   U32   sector_count = (sati_get_cdb_byte(cdb, 6) << 24) |
                        (sati_get_cdb_byte(cdb, 7) << 16) |
                        (sati_get_cdb_byte(cdb, 8) << 8)  |
                        (sati_get_cdb_byte(cdb, 9));

   if(sati_device_state_stopped(sequence, scsi_io))
   {
      return SATI_FAILURE_CHECK_RESPONSE_DATA;
   }
   else
   {
      sequence->type = SATI_SEQUENCE_WRITE_12;

      return sati_write_32_bit_lba_translate_command(
                sequence, scsi_io, ata_io, sector_count, 11
             );
   }
}

/**
 * @brief This method will translate the SCSI write 16 command into a
 *        corresponding ATA write command.  Depending upon the capabilities
 *        supported by the target different ATA commands can be selected.
 *        It ensures that all translation required for this command is
 *        performed successfully.
 *        For more information on the parameters passed to this method,
 *        please reference sati_translate_command().
 *
 * @return Indicate if the command translation succeeded.
 * @see sati_write_large_translate_command(), sati_move_translate_64_bit_lba()
 *      for additional return values.
 */
SATI_STATUS sati_write_16_translate_command(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * scsi_io,
   void                       * ata_io
)
{
   SATI_STATUS   status;
   U8            device_head  = 0;
   U8          * cdb          = sati_cb_get_cdb_address(scsi_io);
   U32           sector_count = (sati_get_cdb_byte(cdb, 10) << 24) |
                                (sati_get_cdb_byte(cdb, 11) << 16) |
                                (sati_get_cdb_byte(cdb, 12) << 8)  |
                                (sati_get_cdb_byte(cdb, 13));

   if(sati_device_state_stopped(sequence, scsi_io))
   {
      return SATI_FAILURE_CHECK_RESPONSE_DATA;
   }
   else
   {
      sequence->type   = SATI_SEQUENCE_WRITE_16;

      // Translate the sector count, write command register, and check various
      // other parts of the CDB.
      status = sati_write_large_translate_command(
                  sequence, scsi_io, ata_io, sector_count, &device_head
               );

      // Attempt to translate the 64-bit LBA field from the SCSI request
      // into the 48-bits of LBA in the ATA register FIS.
      if (status == SATI_SUCCESS)
      {
         sati_move_translate_command(sequence, scsi_io, ata_io, device_head);
         status = sati_move_translate_64_bit_lba(sequence, scsi_io, ata_io);
      }

      return status;
   }
}