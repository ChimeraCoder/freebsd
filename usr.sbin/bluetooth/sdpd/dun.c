
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

#include <sys/queue.h>
#include <bluetooth.h>
#include <sdp.h>
#include <string.h>
#include "profile.h"
#include "provider.h"

static int32_t
dun_profile_create_service_class_id_list(
		uint8_t *buf, uint8_t const * const eob,
		uint8_t const *data, uint32_t datalen)
{
	static uint16_t	service_classes[] = {
		SDP_SERVICE_CLASS_DIALUP_NETWORKING
	};

	return (common_profile_create_service_class_id_list(
			buf, eob,
			(uint8_t const *) service_classes,
			sizeof(service_classes)));
}

static int32_t
dun_profile_create_bluetooth_profile_descriptor_list(
		uint8_t *buf, uint8_t const * const eob,
		uint8_t const *data, uint32_t datalen)
{
	static uint16_t	profile_descriptor_list[] = {
		SDP_SERVICE_CLASS_DIALUP_NETWORKING,
		0x0100
	};

	return (common_profile_create_bluetooth_profile_descriptor_list(
			buf, eob,
			(uint8_t const *) profile_descriptor_list,
			sizeof(profile_descriptor_list)));
}

static int32_t
dun_profile_create_service_name(
		uint8_t *buf, uint8_t const * const eob,
		uint8_t const *data, uint32_t datalen)
{
	static char	service_name[] = "DialUp networking";

	return (common_profile_create_string8(
			buf, eob,
			(uint8_t const *) service_name, strlen(service_name)));
}

static int32_t
dun_profile_create_protocol_descriptor_list(
		uint8_t *buf, uint8_t const * const eob,
		uint8_t const *data, uint32_t datalen)
{
	provider_p		provider = (provider_p) data;
	sdp_dun_profile_p	dun = (sdp_dun_profile_p) provider->data;

	return (rfcomm_profile_create_protocol_descriptor_list(
			buf, eob,
			(uint8_t const *) &dun->server_channel, 1)); 
}

static int32_t
dun_profile_create_audio_feedback_support(
		uint8_t *buf, uint8_t const * const eob,
		uint8_t const *data, uint32_t datalen)
{
	provider_p		provider = (provider_p) data;
	sdp_dun_profile_p	dun = (sdp_dun_profile_p) provider->data;

	if (buf + 2 > eob)
		return (-1);

	SDP_PUT8(SDP_DATA_BOOL, buf);
	SDP_PUT8(dun->audio_feedback_support, buf);

	return (2);
}

static attr_t	dun_profile_attrs[] = {
	{ SDP_ATTR_SERVICE_RECORD_HANDLE,
	  common_profile_create_service_record_handle },
	{ SDP_ATTR_SERVICE_CLASS_ID_LIST,
	  dun_profile_create_service_class_id_list },
	{ SDP_ATTR_BLUETOOTH_PROFILE_DESCRIPTOR_LIST,
	  dun_profile_create_bluetooth_profile_descriptor_list },
	{ SDP_ATTR_LANGUAGE_BASE_ATTRIBUTE_ID_LIST,
	  common_profile_create_language_base_attribute_id_list },
	{ SDP_ATTR_PRIMARY_LANGUAGE_BASE_ID + SDP_ATTR_SERVICE_NAME_OFFSET, 
	  dun_profile_create_service_name },
	{ SDP_ATTR_PROTOCOL_DESCRIPTOR_LIST,
	  dun_profile_create_protocol_descriptor_list },
	{ SDP_ATTR_AUDIO_FEEDBACK_SUPPORT,
	  dun_profile_create_audio_feedback_support },
	{ 0, NULL } /* end entry */
};

profile_t	dun_profile_descriptor = {
	SDP_SERVICE_CLASS_DIALUP_NETWORKING,
	sizeof(sdp_dun_profile_t),
	common_profile_server_channel_valid,
	(attr_t const * const) &dun_profile_attrs
};