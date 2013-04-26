
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

#include "includes.h"
#include "common.h"

#include "wps/wps.h"
#include "wps_i.h"

#include "WpsNfcType.h"
#include "WpsNfc.h"


static int init_nfc_pn531(char *path)
{
	u32 ret;

	ret = WpsNfcInit();
	if (ret != WPS_NFCLIB_ERR_SUCCESS) {
		wpa_printf(MSG_ERROR, "WPS (PN531): Failed to initialize "
			   "NFC Library: 0x%08x", ret);
		return -1;
	}

	ret = WpsNfcOpenDevice((int8 *) path);
	if (ret != WPS_NFCLIB_ERR_SUCCESS) {
		wpa_printf(MSG_ERROR, "WPS (PN531): Failed to open "
			   "NFC Device(%s): 0x%08x", path, ret);
		goto fail;
	}

	ret = WpsNfcTokenDiscovery();
	if (ret != WPS_NFCLIB_ERR_SUCCESS) {
		wpa_printf(MSG_ERROR, "WPS (PN531): Failed to discover "
			   "token: 0x%08x", ret);
		WpsNfcCloseDevice();
		goto fail;
	}

	return 0;

fail:
	WpsNfcDeinit();
	return -1;
}


static void * read_nfc_pn531(size_t *size)
{
	uint32 len;
	u32 ret;
	int8 *data;

	ret = WpsNfcRawReadToken(&data, &len);
	if (ret != WPS_NFCLIB_ERR_SUCCESS) {
		wpa_printf(MSG_ERROR, "WPS (PN531): Failed to read: 0x%08x",
			   ret);
		return NULL;
	}

	*size = len;
	return data;
}


static int write_nfc_pn531(void *data, size_t len)
{
	u32 ret;

	ret = WpsNfcRawWriteToken(data, len);
	if (ret != WPS_NFCLIB_ERR_SUCCESS) {
		wpa_printf(MSG_ERROR, "WPS (PN531): Failed to write: 0x%08x",
			   ret);
		return -1;
	}

	return 0;
}


static void deinit_nfc_pn531(void)
{
	u32 ret;

	ret = WpsNfcCloseDevice();
	if (ret != WPS_NFCLIB_ERR_SUCCESS)
		wpa_printf(MSG_ERROR, "WPS (PN531): Failed to close "
			   "NFC Device: 0x%08x", ret);

	ret = WpsNfcDeinit();
	if (ret != WPS_NFCLIB_ERR_SUCCESS)
		wpa_printf(MSG_ERROR, "WPS (PN531): Failed to deinitialize "
			   "NFC Library: 0x%08x", ret);
}


struct oob_nfc_device_data oob_nfc_pn531_device_data = {
	.init_func	= init_nfc_pn531,
	.read_func	= read_nfc_pn531,
	.write_func	= write_nfc_pn531,
	.deinit_func	= deinit_nfc_pn531,
};