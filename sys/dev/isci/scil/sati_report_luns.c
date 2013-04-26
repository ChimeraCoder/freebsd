
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
 *        translate the SCSI REPORT LUNS command.
 */

#if !defined(DISABLE_SATI_REPORT_LUNS)

#include <dev/isci/scil/sati_report_luns.h>
#include <dev/isci/scil/sati_callbacks.h>
#include <dev/isci/scil/sati_util.h>
#include <dev/isci/scil/intel_ata.h>
#include <dev/isci/scil/intel_scsi.h>

/**
 * @brief This method will translate the REPORT LUN SCSI command.  This
 *        command is immediately completed, since there is no applicable
 *        ATA/ATAPI command.  The data payload is written and SATI_COMPLETE
 *        is always returned.
 *        For more information on the parameters passed to this method,
 *        please reference sati_translate_command().
 *
 * @return This method always indicates the translation is complete.
 * @retval SATI_COMPLETE This value is always returned.
 */
SATI_STATUS sati_report_luns_translate_command(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * scsi_io,
   void                       * ata_io
)
{
   U8 * cdb = sati_cb_get_cdb_address(scsi_io);

   // Set the data length based on the allocation length field in the CDB.
   sequence->allocation_length = (sati_get_cdb_byte(cdb, 6) << 24) |
                                 (sati_get_cdb_byte(cdb, 7) << 16) |
                                 (sati_get_cdb_byte(cdb, 8) << 8)  |
                                 (sati_get_cdb_byte(cdb, 9));

   // The first 4 bytes indicate the length of the LUN list.  Each
   // LUN entry is 8 bytes.  There is only ever LUN 0 for ATA/ATAPI
   // devices.  The value reported is: n-7, where n is the last byte
   // offset (i.e. 15) in the REPORT LUN data.
   sati_set_data_byte(sequence, scsi_io, 0, 0);
   sati_set_data_byte(sequence, scsi_io, 1, 0);
   sati_set_data_byte(sequence, scsi_io, 2, 0);
   sati_set_data_byte(sequence, scsi_io, 3, 8);

   // Bytes 4-7 are reserved.
   sati_set_data_byte(sequence, scsi_io, 4, 0);
   sati_set_data_byte(sequence, scsi_io, 5, 0);
   sati_set_data_byte(sequence, scsi_io, 6, 0);
   sati_set_data_byte(sequence, scsi_io, 7, 0);

   // Add in our single LUN of zero.
   sati_set_data_byte(sequence, scsi_io, 8, 0);
   sati_set_data_byte(sequence, scsi_io, 9, 0);
   sati_set_data_byte(sequence, scsi_io, 10, 0);
   sati_set_data_byte(sequence, scsi_io, 11, 0);
   sati_set_data_byte(sequence, scsi_io, 12, 0);
   sati_set_data_byte(sequence, scsi_io, 13, 0);
   sati_set_data_byte(sequence, scsi_io, 14, 0);
   sati_set_data_byte(sequence, scsi_io, 15, 0);

   return SATI_COMPLETE;
}

#endif // !defined(DISABLE_SATI_REPORT_LUNS)