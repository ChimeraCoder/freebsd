
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
 *        translate the SCSI lun reset command.
 */

#if !defined(DISABLE_SATI_TASK_MANAGEMENT)

#include <dev/isci/scil/sati_lun_reset.h>
#include <dev/isci/scil/sati_callbacks.h>
#include <dev/isci/scil/sati_util.h>
#include <dev/isci/scil/intel_ata.h>
#include <dev/isci/scil/intel_scsi.h>
#include <dev/isci/scil/intel_sat.h>

//******************************************************************************
//* P U B L I C   M E T H O D S
//******************************************************************************

/**
 * @brief This method will translate the lun reset SCSI task request into an
 *        ATA SOFT RESET command. For more information on the parameters
 *        passed to this method, please reference sati_translate_command().
 *
 * @return Indicate if the command translation succeeded.
 * @retval SCI_SUCCESS This is returned if the command translation was
 *         successful.
 *
 * @note It is up to the user of the sata translator to set the command bit
 *       and clear the control softreset bit and send the second register fis.
 */
SATI_STATUS sati_lun_reset_translate_command(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * scsi_io,
   void                       * ata_io
)
{
   U8* register_fis = sati_cb_get_h2d_register_fis_address(ata_io);

   sati_set_ata_command(register_fis, ATA_NOP);
   sati_set_ata_control(register_fis, ATA_CONTROL_REG_SOFT_RESET_BIT);

   //set all other fields to zero.
   sati_clear_sata_command_flag(register_fis);
   sati_set_ata_features(register_fis, 0);
   sati_set_ata_features_exp(register_fis, 0);
   sati_set_ata_sector_count(register_fis, 0);
   sati_set_ata_sector_count_exp(register_fis, 0);
   sati_set_ata_lba_low(register_fis, 0);
   sati_set_ata_lba_mid(register_fis, 0);
   sati_set_ata_lba_high(register_fis, 0);
   sati_set_ata_lba_low_exp(register_fis, 0);
   sati_set_ata_lba_mid_exp(register_fis, 0);
   sati_set_ata_lba_high_exp(register_fis, 0);
   sati_set_ata_device_head(register_fis, 0);

   sequence->type                = SATI_SEQUENCE_LUN_RESET;
   sequence->data_direction      = SATI_DATA_DIRECTION_NONE;
   sequence->protocol            = SAT_PROTOCOL_SOFT_RESET;
   sequence->ata_transfer_length = 0;

   return SATI_SUCCESS;
}

#endif // !defined(DISABLE_SATI_TASK_MANAGEMENT)