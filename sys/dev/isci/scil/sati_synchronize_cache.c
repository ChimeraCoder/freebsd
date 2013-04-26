
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
 * @brief This file contains the method implementations for translating
 *        the SCSI SYNCHRONIZE_CACHE (10, 16-byte) commands.
 */

#if !defined(DISABLE_SATI_SYNCHRONIZE_CACHE)

#include <dev/isci/scil/sati_synchronize_cache.h>
#include <dev/isci/scil/sati_callbacks.h>
#include <dev/isci/scil/sati_util.h>
#include <dev/isci/scil/sati_translator_sequence.h>

#include <dev/isci/scil/intel_ata.h>
#include <dev/isci/scil/intel_scsi.h>
#include <dev/isci/scil/intel_sat.h>

//******************************************************************************
//* P R I V A T E   M E T H O D S
//******************************************************************************

/**
 * @brief This method performs the SCSI SYNCHRONIZE_CACHE 10 and 16 command translation
 *        functionality common to all SYNCHRONIZE_CACHE command sizes.
 *        This includes:
 *        - setting the command register
 *        - setting the device head register
 *        - filling in fields in the SATI_TRANSLATOR_SEQUENCE object.
 *        For more information on the parameters passed to this method,
 *        please reference sati_translate_command().
 *
 * @return Indicate if the method was successfully completed.
 * @retval SATI_SUCCESS This is returned in all other cases.
 */
SATI_STATUS sati_synchronize_cache_translate_command(
   SATI_TRANSLATOR_SEQUENCE_T * sequence,
   void                       * scsi_io,
   void                       * ata_io
)
{
   U8 * cdb          = sati_cb_get_cdb_address(scsi_io);
   U8 * register_fis = sati_cb_get_h2d_register_fis_address(ata_io);

   sequence->type           = SATI_SEQUENCE_SYNCHRONIZE_CACHE;
   sequence->protocol       = SAT_PROTOCOL_NON_DATA;
   sequence->data_direction = SATI_DATA_DIRECTION_NONE;

   if (sati_get_cdb_byte(cdb, 1) & SCSI_SYNCHRONIZE_CACHE_IMMED_ENABLED)
   {
      //currently we ignore immed bit.
      ;
   }

   // Ensure the device supports the 48 bit feature set.
   if (sequence->device->capabilities & SATI_DEVICE_CAP_48BIT_ENABLE)
      sati_set_ata_command(register_fis, ATA_FLUSH_CACHE_EXT);
   else
      sati_set_ata_command(register_fis, ATA_FLUSH_CACHE);

   return SATI_SUCCESS;
}

#endif // !defined(DISABLE_SATI_SYNCHRONIZE_CACHE)