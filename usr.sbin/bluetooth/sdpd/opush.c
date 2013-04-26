
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
opush_profile_create_service_class_id_list(
		uint8_t *buf, uint8_t const * const eob,
		uint8_t const *data, uint32_t datalen)
{
	static uint16_t	service_classes[] = {
		SDP_SERVICE_CLASS_OBEX_OBJECT_PUSH
	};

	return (common_profile_create_service_class_id_list(
			buf, eob,
			(uint8_t const *) service_classes,
			sizeof(service_classes)));
}

static int32_t
opush_profile_create_bluetooth_profile_descriptor_list(
		uint8_t *buf, uint8_t const * const eob,
		uint8_t const *data, uint32_t datalen)
{
	static uint16_t	profile_descriptor_list[] = {
		SDP_SERVICE_CLASS_OBEX_OBJECT_PUSH,
		0x0100
	};

	return (common_profile_create_bluetooth_profile_descriptor_list(
			buf, eob,
			(uint8_t const *) profile_descriptor_list,
			sizeof(profile_descriptor_list)));
}

static int32_t
opush_profile_create_service_name(
		uint8_t *buf, uint8_t const * const eob,
		uint8_t const *data, uint32_t datalen)
{
	static char	service_name[] = "OBEX Object Push";

	return (common_profile_create_string8(
			buf, eob,
			(uint8_t const *) service_name, strlen(service_name)));
}

static int32_t
opush_profile_create_protocol_descriptor_list(
		uint8_t *buf, uint8_t const * const eob,
		uint8_t const *data, uint32_t datalen)
{
	provider_p		provider = (provider_p) data;
	sdp_opush_profile_p	opush = (sdp_opush_profile_p) provider->data;

	return (obex_profile_create_protocol_descriptor_list(
			buf, eob,
			(uint8_t const *) &opush->server_channel, 1)); 
}

static int32_t
opush_profile_create_supported_formats_list(
		uint8_t *buf, uint8_t const * const eob,
		uint8_t const *data, uint32_t datalen)
{
	provider_p		provider = (provider_p) data;
	sdp_opush_profile_p	opush = (sdp_opush_profile_p) provider->data;

	return (obex_profile_create_supported_formats_list(
			buf, eob,
			(uint8_t const *) opush->supported_formats,
			opush->supported_formats_size));
}

static attr_t	opush_profile_attrs[] = {
	{ SDP_ATTR_SERVICE_RECORD_HANDLE,
	  common_profile_create_service_record_handle },
	{ SDP_ATTR_SERVICE_CLASS_ID_LIST,
	  opush_profile_create_service_class_id_list },
	{ SDP_ATTR_BLUETOOTH_PROFILE_DESCRIPTOR_LIST,
	  opush_profile_create_bluetooth_profile_descriptor_list },
	{ SDP_ATTR_LANGUAGE_BASE_ATTRIBUTE_ID_LIST,
	  common_profile_create_language_base_attribute_id_list },
	{ SDP_ATTR_PRIMARY_LANGUAGE_BASE_ID + SDP_ATTR_SERVICE_NAME_OFFSET,
	  opush_profile_create_service_name },
	{ SDP_ATTR_PROTOCOL_DESCRIPTOR_LIST,
	  opush_profile_create_protocol_descriptor_list },
	{ SDP_ATTR_SUPPORTED_FORMATS_LIST,
	  opush_profile_create_supported_formats_list },
	{ 0, NULL } /* end entry */
};

profile_t	opush_profile_descriptor = {
	SDP_SERVICE_CLASS_OBEX_OBJECT_PUSH,
	sizeof(sdp_opush_profile_t),
	obex_profile_data_valid,
	(attr_t const * const) &opush_profile_attrs
};