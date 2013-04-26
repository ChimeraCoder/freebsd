
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
 * @brief This file contains the method implementations required to
 *        translate the SCSI abort task set command.
 */

#if !defined(DISABLE_SATI_TASK_MANAGEMENT)

#include <dev/isci/scil/sati_abort_task_set.h>
#include <dev/isci/scil/sati_callbacks.h>
#include <dev/isci/scil/sati_util.h>
#include <dev/isci/scil/sati.h>
#include <dev/isci/scil/intel_ata.h>
#include <dev/isci/scil/intel_scsi.h>
#include <dev/isci/scil/intel_sat.h>

//******************************************************************************
//* P U B L I C   M E T H O D S
//******************************************************************************

#if !defined(DISABLE_SATI_ABORT_TASK_SET)

/**
 * @brief This method will translate the abort task set SCSI task request into an
 *        ATA READ LOG EXT command. For more information on the parameters
 *        passed to this method, please reference sati_translate_command().
 *
 * @return Indicate if the command translation succeeded.
 * @retval SCI_SUCCESS This is returned if the command translation was
 *         successful.
 */
SATI_STATUS sati_abort_task_set_translate_command(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * scsi_io,
   void                       * ata_io
)
{
   U8 * register_fis;

   //ATA Read Log Ext with log address set to 0x10
   sati_ata_read_log_ext_construct(
      ata_io,
      sequence,
      ATA_LOG_PAGE_NCQ_ERROR,
      sizeof(ATA_NCQ_COMMAND_ERROR_LOG_T)
   );

   register_fis = sati_cb_get_h2d_register_fis_address(ata_io);
   sati_set_sata_command_flag(register_fis);

   sequence->type                = SATI_SEQUENCE_ABORT_TASK_SET;
   sequence->state               = SATI_SEQUENCE_STATE_AWAIT_RESPONSE;

   return SATI_SUCCESS;
}

SATI_STATUS sati_abort_task_set_translate_data(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * ata_input_data,
   void                       * scsi_task
)
{
   ATA_NCQ_COMMAND_ERROR_LOG_T * log =
      (ATA_NCQ_COMMAND_ERROR_LOG_T *)ata_input_data;
   U8 tag_index;

   sequence->state = SATI_SEQUENCE_STATE_TRANSLATE_DATA;

   for (tag_index = 0; tag_index < 32; tag_index++)
   {
      void *        matching_command;
      SCI_IO_STATUS completion_status;
      sati_cb_device_get_request_by_ncq_tag(
         scsi_task,
         tag_index,
         matching_command
      );

      if (matching_command != NULL)
      {
         if (
              (log->ncq_tag == tag_index) &&
              (log->nq == 0) // nq==1 means a non-queued command
                             //  caused this failure
            )
         {
            sati_translate_error(sequence, matching_command, log->error);
            completion_status = SCI_IO_FAILURE_RESPONSE_VALID;

            if(sequence->state == SATI_SEQUENCE_STATE_READ_ERROR)
            {
               //Uncorrectable read error, return additional sense data
               sati_scsi_read_ncq_error_sense_construct(
                  sequence,
                  matching_command,
                  ata_input_data,
                  SCSI_STATUS_CHECK_CONDITION,
                  SCSI_SENSE_MEDIUM_ERROR,
                  SCSI_ASC_UNRECOVERED_READ_ERROR,
                  SCSI_ASCQ_UNRECOVERED_READ_ERROR
               );
            }
         }
         else
         {
            completion_status = SCI_IO_FAILURE_TERMINATED;
         }

         sati_cb_io_request_complete(matching_command, completion_status);
      }
   }

   sequence->state = SATI_SEQUENCE_STATE_FINAL;

   return SATI_COMPLETE;
}

#endif // !defined(DISABLE_SATI_ABORT_TASK_SET)

#endif // !defined(DISABLE_SATI_TASK_MANAGEMENT)