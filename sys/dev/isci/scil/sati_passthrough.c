
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
 *        translate the SCSI passthru command.
 */

#if !defined(DISABLE_SATI_PASSTHROUGH)

#include <dev/isci/scil/sati.h>
#include <dev/isci/scil/sati_passthrough.h>
#include <dev/isci/scil/sati_util.h>
#include <dev/isci/scil/sati_callbacks.h>
#include <dev/isci/scil/intel_ata.h>

#define PASSTHROUGH_CDB_PROTOCOL_MASK       0x1E
#define PASSTHROUGH_CDB_EXTEND_MASK         0x1
#define PASSTHROUGH_CDB_CK_COND_MASK        0x20
#define PASSTHROUGH_CDB_T_DIR_MASK          0x8

#define PASSTHROUGH_ISOLATE_BITS(cdb, index, mask, shift) (((sati_get_cdb_byte(cdb, index) & mask) >> shift))

#define PASSTHROUGH_CDB_PROTOCOL(cdb)      PASSTHROUGH_ISOLATE_BITS(cdb, 1, PASSTHROUGH_CDB_PROTOCOL_MASK, 1)
#define PASSTHROUGH_CDB_EXTEND(cdb)        PASSTHROUGH_ISOLATE_BITS(cdb, 1, PASSTHROUGH_CDB_EXTEND_MASK, 0)
#define PASSTHROUGH_CDB_CK_COND(cdb)       PASSTHROUGH_ISOLATE_BITS(cdb, 2, PASSTHROUGH_CDB_CK_COND_MASK, 5)
#define PASSTHROUGH_CDB_T_DIR(cdb)         PASSTHROUGH_ISOLATE_BITS(cdb, 2, PASSTHROUGH_CDB_T_DIR_MASK, 3)

#define PASSTHROUGH_CDB_MULTIPLE_COUNT(cdb)         (sati_get_cdb_byte(cdb, 1) >> 5)
#define PASSTHROUGH_CDB_COMMAND(cdb, index)         sati_get_cdb_byte(cdb, index)

// Protocols
#define PASSTHROUGH_PIO_DATA_IN            0x4
#define PASSTHROUGH_PIO_DATA_OUT           0x5
#define PASSTHROUGH_UDMA_DATA_IN           0xA
#define PASSTHROUGH_UDMA_DATA_OUT          0xB
#define PASSTHROUGH_RETURN_RESPONSE        0xF

/**
* @brief This function will check the multiple_count field in the SCSI CDB
*        and if multiple_count is nonzero the function will check the
*        ATA command code. Only Read and Write Multiple commands are allowed
*        when multiple_count is a nonzero value.
*
* @param[in]     cdb The SCSI cdb for the ATA pass-through command
*
* @return BOOL
  @retval TRUE - multiple_count is nonzero with a unsupported command
  @retval FALSE - multiple_count is zero or the command supports a nonzero value
*/
static
BOOL sati_passthrough_multiple_count_error(
   U8 *     cdb
)
{
   U8 ata_command_code;

   if(PASSTHROUGH_CDB_MULTIPLE_COUNT(cdb) > 0)
   {
      if(sati_get_cdb_byte(cdb, 0 ) == SCSI_ATA_PASSTHRU_12)
      {
         ata_command_code = PASSTHROUGH_CDB_COMMAND(cdb, 9);
      }
      else
      {
         ata_command_code = PASSTHROUGH_CDB_COMMAND(cdb, 14);
      }

      switch(ata_command_code)
      {  //MULTICOUNT bit is supported
         case ATA_READ_MULTIPLE:
         case ATA_READ_MULTIPLE_EXT:
         case ATA_WRITE_MULTIPLE:
         case ATA_WRITE_MULTIPLE_EXT:
         case ATA_WRITE_MULTIPLE_FUA_EXT:
            return FALSE;
         break;

         default:
            return TRUE;
      }
   }
   //MULTICOUNT bit is not set
   return FALSE;
}

/**
 * @brief This method will construct the sense data buffer in the user's
 *        sense data buffer location.  Additionally, it will set the user's
 *        SCSI status.
 *
 * @param[in]     sequence This parameter specifies the translation sequence
 *                for which to construct the sense data.
 * @param[in]     register_fis This parameter specifies the fis from which
 *                to get the data.
 * @param[in,out] scsi_io This parameter specifies the user's IO request
 *                for which to construct the sense data.
 * @param[in]     scsi_status This parameter specifies the SCSI status
 *                value for the user's IO request.
 * @param[in]     sense_key This parameter specifies the sense key to
 *                be set for the user's IO request.
 * @param[in]     additional_sense_code This parameter specifies the
 *                additional sense code (ASC) key to be set for the user's
 *                IO request.
 * @param[in]     additional_sense_code_qualifier This parameter specifies
 *                the additional sense code qualifier (ASCQ) key to be set
 *                for the user's IO request.
 *
 * @return none
 */
static
void sati_passthrough_construct_sense(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   U8                         * register_fis,
   void                       * scsi_io,
   U8                           scsi_status,
   U8                           sense_key,
   U8                           additional_sense_code,
   U8                           additional_sense_code_qualifier
)
{
   U8                    * sense_data;
   U32                     sense_len;
   U8                    * cdb;
   unsigned char           sector_count_upper;
   unsigned char           lba_upper;

#ifdef SATI_TRANSPORT_SUPPORTS_SAS
   SCI_SSP_RESPONSE_IU_T * rsp_iu = (SCI_SSP_RESPONSE_IU_T*)
                                    sati_cb_get_response_iu_address(scsi_io);

   sati_scsi_common_response_iu_construct(
      rsp_iu,
      scsi_status,
      SCSI_FIXED_SENSE_DATA_BASE_LENGTH,
      SCSI_RESPONSE_DATA_PRES_SENSE_DATA
   );

   sense_data                   = (U8*) rsp_iu->data;
   sense_len                    = SSP_RESPONSE_IU_MAX_DATA * 4;  // dwords to bytes
#else
   sense_data = sati_cb_get_sense_data_address(scsi_io);
   sense_len  = sati_cb_get_sense_data_length(scsi_io);
#endif // SATI_TRANSPORT_SUPPORTS_SAS

   sati_scsi_sense_data_construct(
      sequence,
      scsi_io,
      scsi_status,
      sense_key,
      additional_sense_code,
      additional_sense_code_qualifier
   );

   cdb = sati_cb_get_cdb_address(scsi_io);

   if (sati_get_ata_sector_count_ext(register_fis) != 0) {
      sector_count_upper = 1;
   } else {
       sector_count_upper = 0;
   }

   if (sati_get_ata_lba_high_ext(register_fis) != 0 ||
       sati_get_ata_lba_mid_ext(register_fis) != 0 ||
       sati_get_ata_lba_low_ext(register_fis) != 0) {
      lba_upper = 1;
   } else {
       lba_upper = 0;
   }

   // Information section
   sati_set_sense_data_byte(sense_data, sense_len, 3,  (U8)sati_get_ata_error(register_fis));
   sati_set_sense_data_byte(sense_data, sense_len, 4,  (U8)sati_get_ata_status(register_fis));
   sati_set_sense_data_byte(sense_data, sense_len, 5,  sati_get_ata_device(register_fis));
   sati_set_sense_data_byte(sense_data, sense_len, 6,  sati_get_ata_sector_count(register_fis));

   // Command specific section
   sati_set_sense_data_byte(sense_data, sense_len, 8,  (PASSTHROUGH_CDB_EXTEND(cdb) << 7) | (sector_count_upper << 6) | (lba_upper << 5));
   sati_set_sense_data_byte(sense_data, sense_len, 9,  sati_get_ata_lba_high(register_fis));
   sati_set_sense_data_byte(sense_data, sense_len, 10, sati_get_ata_lba_mid(register_fis));
   sati_set_sense_data_byte(sense_data, sense_len, 11, sati_get_ata_lba_low(register_fis));

   sequence->is_sense_response_set = TRUE;
}

/**
 * @brief This method will verify that the T_DIR bit matches the protocol bit.
 *        It will additionally set the direction on the sequence.
 *
 * @param[in,out] sequence This parameter specifies the translation sequence
 *                for which to construct the sense data.
 * @param[in]     cdb The CDB containing the passthrough command
 *
 * @return none
 */
static
SATI_STATUS sati_passthrough_check_direction(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   U8           * cdb
)
{
   if ((PASSTHROUGH_CDB_PROTOCOL(cdb) == PASSTHROUGH_PIO_DATA_IN) ||
       (PASSTHROUGH_CDB_PROTOCOL(cdb) == PASSTHROUGH_UDMA_DATA_IN))
   {
      if (PASSTHROUGH_CDB_T_DIR(cdb) == 0x0)
      {
         return SATI_FAILURE;
      }
      else
      {
         sequence->data_direction = SATI_DATA_DIRECTION_IN;
      }
   }
   else if ((PASSTHROUGH_CDB_PROTOCOL(cdb) == PASSTHROUGH_PIO_DATA_OUT) ||
            (PASSTHROUGH_CDB_PROTOCOL(cdb) == PASSTHROUGH_UDMA_DATA_OUT))
   {
      if (PASSTHROUGH_CDB_T_DIR(cdb) == 0x1)
      {
         return SATI_FAILURE;
      }
      else
      {
         sequence->data_direction = SATI_DATA_DIRECTION_OUT;
      }
   }
   else
   {
      sequence->data_direction = SATI_DATA_DIRECTION_NONE;
   }

   return SATI_COMPLETE;
}

//******************************************************************************
//* P U B L I C   M E T H O D S
//******************************************************************************

/**
 * @brief This method will translate the SCSI Passthrough command
 *        into the corresponding ATA command.
 *
 * @return Indicate if the command translation succeeded.
 * @retval SATI_SUCCESS This is returned if the command translation was
 *         successful.
 * @retval SATI_FAILURE This is returned if the command translation was
 *         unsuccessful
 */

SATI_STATUS sati_passthrough_12_translate_command(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * scsi_io,
   void                       * ata_io
)
{
   SATI_STATUS   status;
   U8          * cdb;
   U8          * register_fis;

   status = SATI_FAILURE;

   sequence->type = SATI_SEQUENCE_ATA_PASSTHROUGH_12;
   sequence->state = SATI_SEQUENCE_STATE_TRANSLATE_DATA;

   cdb = sati_cb_get_cdb_address(scsi_io);
   sequence->protocol = PASSTHROUGH_CDB_PROTOCOL (cdb);
   register_fis = sati_cb_get_h2d_register_fis_address(ata_io);

   if (sati_passthrough_check_direction(sequence, cdb) != SATI_COMPLETE
       || sati_passthrough_multiple_count_error(cdb)
      )
   {
      // Fail due to mismatch
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

   sati_set_ata_features(register_fis, sati_get_cdb_byte(cdb, 3));
   sati_set_ata_sector_count(register_fis, sati_get_cdb_byte(cdb, 4));
   sati_set_ata_lba_low(register_fis, sati_get_cdb_byte(cdb, 5));
   sati_set_ata_lba_mid(register_fis, sati_get_cdb_byte(cdb, 6));
   sati_set_ata_lba_high(register_fis, sati_get_cdb_byte(cdb, 7));
   sati_set_ata_device_head(register_fis, sati_get_cdb_byte(cdb, 8));
   sati_set_ata_command(register_fis, sati_get_cdb_byte(cdb, 9));

   sequence->state = SATI_SEQUENCE_STATE_AWAIT_RESPONSE;

   return SATI_SUCCESS;
}

/**
 * @brief This method will translate the SCSI Passthrough command
 *        into the corresponding ATA command.
 *
 * @return Indicate if the command translation succeeded.
 * @retval SATI_SUCCESS This is returned if the command translation was
 *         successful.
 * @retval SATI_FAILURE This is returned if the command translation was
 *         unsuccessful
 */
SATI_STATUS sati_passthrough_16_translate_command(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * scsi_io,
   void                       * ata_io
)
{
   SATI_STATUS   status;
   U8          * cdb;
   U8          * register_fis;

   status = SATI_FAILURE;

   sequence->type = SATI_SEQUENCE_ATA_PASSTHROUGH_16;
   sequence->state = SATI_SEQUENCE_STATE_TRANSLATE_DATA;

   cdb = sati_cb_get_cdb_address(scsi_io);
   sequence->protocol = PASSTHROUGH_CDB_PROTOCOL(cdb);
   register_fis = sati_cb_get_h2d_register_fis_address(ata_io);

   if (sati_passthrough_check_direction(sequence, cdb) != SATI_COMPLETE
       || sati_passthrough_multiple_count_error(cdb)
      )
   {
      // Fail due to mismatch
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

   if (PASSTHROUGH_CDB_EXTEND(cdb) == 1)
   {
      sati_set_ata_features_exp(register_fis, sati_get_cdb_byte(cdb, 3));
      sati_set_ata_sector_count_exp(register_fis, sati_get_cdb_byte(cdb, 5));
      sati_set_ata_lba_low_exp(register_fis, sati_get_cdb_byte(cdb, 7));
      sati_set_ata_lba_mid_exp(register_fis, sati_get_cdb_byte(cdb, 9));
      sati_set_ata_lba_high_exp(register_fis, sati_get_cdb_byte(cdb, 11));
   }
   sati_set_ata_features(register_fis, sati_get_cdb_byte(cdb, 4));
   sati_set_ata_sector_count(register_fis, sati_get_cdb_byte(cdb, 6));
   sati_set_ata_lba_low(register_fis, sati_get_cdb_byte(cdb, 8));
   sati_set_ata_lba_mid(register_fis, sati_get_cdb_byte(cdb, 10));
   sati_set_ata_lba_high(register_fis, sati_get_cdb_byte(cdb, 12));
   sati_set_ata_device_head(register_fis, sati_get_cdb_byte(cdb, 13));
   sati_set_ata_command(register_fis, sati_get_cdb_byte(cdb, 14));

   sequence->state = SATI_SEQUENCE_STATE_AWAIT_RESPONSE;

   return SATI_SUCCESS;
}

/**
 * @brief This method will translate the ATA command
 *        response
 *
 * @return Indicate if the command translation succeeded.
 * @retval SATI_COMPLETE This is returned if the command translation was
 *         successful.
 * @retval SATI_FAILURE This is returned if the command translation was
 *         unsuccessful
 */
SATI_STATUS sati_passthrough_translate_response(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * scsi_io,
   void                       * ata_io
)
{
   U8 * cdb;
   U8 * register_fis;

   cdb = sati_cb_get_cdb_address(scsi_io);
   register_fis = sati_cb_get_d2h_register_fis_address(ata_io);

   // Check for device errors
   if (sati_get_ata_status(register_fis) & ATA_STATUS_REG_ERROR_BIT)
   {
      sati_translate_error(sequence, scsi_io, (U8)sati_get_ata_error(register_fis));
      return SATI_FAILURE_CHECK_RESPONSE_DATA;
   }

   // If the user set the check condition bit, fill out the sense data
   if (PASSTHROUGH_CDB_CK_COND(cdb) ||
       PASSTHROUGH_CDB_PROTOCOL(cdb) == PASSTHROUGH_RETURN_RESPONSE)
   {
      sati_passthrough_construct_sense(
         sequence,
         register_fis,
         scsi_io,
         SCSI_STATUS_CHECK_CONDITION,
         SCSI_SENSE_RECOVERED_ERROR,
         SCSI_ASC_NO_ADDITIONAL_SENSE,
         SCSI_ASCQ_ATA_PASS_THROUGH_INFORMATION_AVAILABLE
      );
   }

   sequence->state = SATI_SEQUENCE_STATE_FINAL;

   return SATI_COMPLETE;
}

#endif // !defined(DISABLE_SATI_PASSTHROUGH)