
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
 * @brief This file contains the implementation to translate
 *        SCSI Write Long 10 and 16 commands based on the SAT spec.
 */

#if !defined(DISABLE_SATI_WRITE_LONG)

#include <dev/isci/scil/sati_write_long.h>
#include <dev/isci/scil/sati_device.h>
#include <dev/isci/scil/sati_util.h>
#include <dev/isci/scil/intel_scsi.h>
#include <dev/isci/scil/intel_ata.h>
#include <dev/isci/scil/intel_sat.h>
#include <dev/isci/scil/sati_callbacks.h>
#include <dev/isci/scil/sati_move.h>

#define LOGICAL_PER_PHYSICAL_SECTOR 0xF

#define WR_UNCOR_BIT          0x02
#define WR_UNCOR_PBLOCK_BIT   0x03
#define COR_DIS_WR_UNCORR_BIT 0x06


/**
 * @brief This method will translate the write long 10 & 16 SCSI commands into
 *        ATA write uncorrectable commands. For more information on the
 *        parameters passed to this method, please reference
 *        sati_translate_command().
 *
 * @return Indicate if the command translation succeeded.
 * @retval SCI_SUCCESS This is returned if the command translation was
 *         successful.
 * @retval SATI_FAILURE_CHECK_RESPONSE_DATA is returned if there was
 *         a problem with the translation of write long.
 *
 */
SATI_STATUS sati_write_long_translate_command(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * scsi_io,
   void                       * ata_io
)
{
   U8 * cdb = sati_cb_get_cdb_address(scsi_io);
   SATI_STATUS status = SATI_FAILURE;
   U16 byte_transfer_length;
   U8 device_head  = 0;

   if((sequence->device->capabilities &
       SATI_DEVICE_CAP_WRITE_UNCORRECTABLE_ENABLE) == 0)
   {
      sati_scsi_sense_data_construct(
         sequence,
         scsi_io,
         SCSI_STATUS_CHECK_CONDITION,
         SCSI_SENSE_ILLEGAL_REQUEST,
         SCSI_ASC_INVALID_COMMAND_OPERATION_CODE,
         SCSI_ASCQ_INVALID_COMMAND_OPERATION_CODE
      );
      return SATI_FAILURE_CHECK_RESPONSE_DATA;
   }

   //Write Long 10
   if(sati_get_cdb_byte(cdb, 0) == SCSI_WRITE_LONG_10)
   {
      byte_transfer_length = (sati_get_cdb_byte(cdb, 7) << 8) |
                             (sati_get_cdb_byte(cdb, 8));

      sati_move_translate_32_bit_lba(sequence, scsi_io, ata_io);
   }
   else //Write Long 16
   {
      byte_transfer_length = (sati_get_cdb_byte(cdb, 12) << 8) |
                             (sati_get_cdb_byte(cdb, 13));

      status = sati_move_translate_64_bit_lba(sequence, scsi_io, ata_io);

      if( status == SATI_FAILURE_CHECK_RESPONSE_DATA)
      {
         return status;
      }
   }


   sati_move_translate_command(sequence, scsi_io, ata_io, device_head);

   if( byte_transfer_length != 0 )
   {
      sati_scsi_sense_data_construct(
         sequence,
         scsi_io,
         SCSI_STATUS_CHECK_CONDITION,
         SCSI_SENSE_ILLEGAL_REQUEST,
         SCSI_ASC_INVALID_FIELD_IN_CDB,
         SCSI_ASCQ_INVALID_FIELD_IN_CDB
      );
      return SATI_FAILURE_CHECK_RESPONSE_DATA;
   }

   switch(SATI_WRITE_LONG_GET_COR_WR_PB_BITS(cdb))
   {
      case WR_UNCOR_BIT :

         if( (sequence->device->capabilities &
              SATI_DEVICE_CAP_MULTIPLE_SECTORS_PER_PHYSCIAL_SECTOR) != 0 )
         {
            sati_scsi_sense_data_construct(
               sequence,
               scsi_io,
               SCSI_STATUS_CHECK_CONDITION,
               SCSI_SENSE_ILLEGAL_REQUEST,
               SCSI_ASC_INVALID_FIELD_IN_CDB,
               SCSI_ASCQ_INVALID_FIELD_IN_CDB
            );
            return SATI_FAILURE_CHECK_RESPONSE_DATA;
         }
         else
         {
            sati_ata_write_uncorrectable_construct(
               ata_io,
               sequence,
               ATA_WRITE_UNCORRECTABLE_PSUEDO
            );
            sequence->type = SATI_SEQUENCE_WRITE_LONG;
            status = SATI_SUCCESS;
         }
         break;

      case WR_UNCOR_PBLOCK_BIT :

         sati_ata_write_uncorrectable_construct(
            ata_io,
            sequence,
            ATA_WRITE_UNCORRECTABLE_PSUEDO
         );
         sequence->type = SATI_SEQUENCE_WRITE_LONG;
         status = SATI_SUCCESS;
         break;

      case COR_DIS_WR_UNCORR_BIT :

         sati_ata_write_uncorrectable_construct(
            ata_io,
            sequence,
            ATA_WRITE_UNCORRECTABLE_FLAGGED
         );
         sequence->type = SATI_SEQUENCE_WRITE_LONG;
         status = SATI_SUCCESS;
         break;

      default :

         sati_scsi_sense_data_construct(
            sequence,
            scsi_io,
            SCSI_STATUS_CHECK_CONDITION,
            SCSI_SENSE_ILLEGAL_REQUEST,
            SCSI_ASC_INVALID_FIELD_IN_CDB,
            SCSI_ASCQ_INVALID_FIELD_IN_CDB
         );
         return SATI_FAILURE_CHECK_RESPONSE_DATA;
         break;
   }
   return status;
}

/**
 * @brief This method will translate the response to the SATI Write Long
 *        translation. This response is only error checking the
 *        ATA Write Uncorrectable command.
 *
 * @return SATI_STATUS Indicates if the response translation succeeded.
 * @retval SCI_COMPLETE This is returned if the command translation was
 *         successful.
 * @retval SATI_FAILURE_CHECK_RESPONSE_DATA is returned if there was
 *         a problem with the translation of write long.
 */
SATI_STATUS sati_write_long_translate_response(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * scsi_io,
   void                       * ata_io
)
{
   U8 * register_fis = sati_cb_get_d2h_register_fis_address(ata_io);

   if (sati_get_ata_status(register_fis) & ATA_STATUS_REG_ERROR_BIT)
   {
      sati_scsi_sense_data_construct(
         sequence,
         scsi_io,
         SCSI_STATUS_CHECK_CONDITION,
         SCSI_SENSE_ABORTED_COMMAND,
         SCSI_ASC_COMMAND_SEQUENCE_ERROR,
         SCSI_ASCQ_NO_ADDITIONAL_SENSE
      );
      return SATI_FAILURE_CHECK_RESPONSE_DATA;
   }
   else
   {
      return SATI_COMPLETE;
   }
}

#endif // !defined(DISABLE_SATI_WRITE_LONG)