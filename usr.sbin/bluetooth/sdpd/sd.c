
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
sd_profile_create_service_class_id_list(
		uint8_t *buf, uint8_t const * const eob,
		uint8_t const *data, uint32_t datalen)
{
	static uint16_t	service_classes[] = {
		SDP_SERVICE_CLASS_SERVICE_DISCOVERY_SERVER
	};

	return (common_profile_create_service_class_id_list(
			buf, eob,
			(uint8_t const *) service_classes,
			sizeof(service_classes)));
}

static int32_t
sd_profile_create_bluetooth_profile_descriptor_list(
		uint8_t *buf, uint8_t const * const eob,
		uint8_t const *data, uint32_t datalen)
{
	static uint16_t profile_descriptor_list[] = {
		SDP_SERVICE_CLASS_SERVICE_DISCOVERY_SERVER,
		0x0100
	};

	return (common_profile_create_bluetooth_profile_descriptor_list(
			buf, eob,
			(uint8_t const *) profile_descriptor_list,
			sizeof(profile_descriptor_list)));
}

static int32_t
sd_profile_create_service_id(
		uint8_t *buf, uint8_t const * const eob,
		uint8_t const *data, uint32_t datalen)
{
	if (buf + 3 > eob)
		return (-1);

	/*
	 * The ServiceID is a UUID that universally and uniquely identifies 
	 * the service instance described by the service record. This service
	 * attribute is particularly useful if the same service is described
	 * by service records in more than one SDP server
	 */

	SDP_PUT8(SDP_DATA_UUID16, buf);
	SDP_PUT16(SDP_UUID_PROTOCOL_SDP, buf); /* XXX ??? */

	return (3);
}

static int32_t
sd_profile_create_service_name(
		uint8_t *buf, uint8_t const * const eob,
		uint8_t const *data, uint32_t datalen)
{
	static char	service_name[] = "Bluetooth service discovery";

	return (common_profile_create_string8(
			buf, eob,
			(uint8_t const *) service_name, strlen(service_name)));
}

static int32_t
sd_profile_create_protocol_descriptor_list(
		uint8_t *buf, uint8_t const * const eob,
		uint8_t const *data, uint32_t datalen)
{
	if (buf + 12 > eob)
		return (-1);

	SDP_PUT8(SDP_DATA_SEQ8, buf);
	SDP_PUT8(10, buf);

	SDP_PUT8(SDP_DATA_SEQ8, buf);
	SDP_PUT8(3, buf);
	SDP_PUT8(SDP_DATA_UUID16, buf);
	SDP_PUT16(SDP_UUID_PROTOCOL_L2CAP, buf);

	SDP_PUT8(SDP_DATA_SEQ8, buf);
	SDP_PUT8(3, buf);
	SDP_PUT8(SDP_DATA_UUID16, buf);
	SDP_PUT16(SDP_UUID_PROTOCOL_SDP, buf);

	return (12);
}

static int32_t
sd_profile_create_browse_group_list(
		uint8_t *buf, uint8_t const * const eob,
		uint8_t const *data, uint32_t datalen)
{
	if (buf + 5 > eob)
		return (-1);

	SDP_PUT8(SDP_DATA_SEQ8, buf);
	SDP_PUT8(3, buf);

	/*
	 * The top-level browse group ID, called PublicBrowseRoot and
	 * representing the root of the browsing hierarchy, has the value
	 * 00001002-0000-1000-8000-00805F9B34FB (UUID16: 0x1002) from the
	 * Bluetooth Assigned Numbers document
	 */

	SDP_PUT8(SDP_DATA_UUID16, buf);
	SDP_PUT16(SDP_SERVICE_CLASS_PUBLIC_BROWSE_GROUP, buf);

	return (5);
}

static int32_t
sd_profile_create_version_number_list(
		uint8_t *buf, uint8_t const * const eob,
		uint8_t const *data, uint32_t datalen)
{
	if (buf + 5 > eob)
		return (-1);

	SDP_PUT8(SDP_DATA_SEQ8, buf);
	SDP_PUT8(3, buf);

	/* 
	 * The VersionNumberList is a data element sequence in which each 
	 * element of the sequence is a version number supported by the SDP
	 * server. A version number is a 16-bit unsigned integer consisting
	 * of two fields. The higher-order 8 bits contain the major version
	 * number field and the low-order 8 bits contain the minor version
	 * number field. The initial version of SDP has a major version of
	 * 1 and a minor version of 0
	 */

	SDP_PUT8(SDP_DATA_UINT16, buf);
	SDP_PUT16(0x0100, buf);

	return (5);
}

static int32_t
sd_profile_create_service_database_state(
		uint8_t *buf, uint8_t const * const eob,
		uint8_t const *data, uint32_t datalen)
{
	uint32_t	change_state = provider_get_change_state();

	if (buf + 5 > eob)
		return (-1);

	SDP_PUT8(SDP_DATA_UINT32, buf);
	SDP_PUT32(change_state, buf);

	return (5);
}

static attr_t	sd_profile_attrs[] = {
	{ SDP_ATTR_SERVICE_RECORD_HANDLE,
	  common_profile_create_service_record_handle },
	{ SDP_ATTR_SERVICE_CLASS_ID_LIST,
	  sd_profile_create_service_class_id_list },
	{ SDP_ATTR_BLUETOOTH_PROFILE_DESCRIPTOR_LIST,
	  sd_profile_create_bluetooth_profile_descriptor_list },
	{ SDP_ATTR_SERVICE_ID,
	  sd_profile_create_service_id },
	{ SDP_ATTR_LANGUAGE_BASE_ATTRIBUTE_ID_LIST,
	  common_profile_create_language_base_attribute_id_list },
	{ SDP_ATTR_PRIMARY_LANGUAGE_BASE_ID + SDP_ATTR_SERVICE_NAME_OFFSET, 
	  sd_profile_create_service_name },
	{ SDP_ATTR_PRIMARY_LANGUAGE_BASE_ID + SDP_ATTR_SERVICE_DESCRIPTION_OFFSET, 
	  sd_profile_create_service_name },
	{ SDP_ATTR_PRIMARY_LANGUAGE_BASE_ID + SDP_ATTR_PROVIDER_NAME_OFFSET,
	  common_profile_create_service_provider_name },
	{ SDP_ATTR_PROTOCOL_DESCRIPTOR_LIST,
	  sd_profile_create_protocol_descriptor_list },
	{ SDP_ATTR_BROWSE_GROUP_LIST,
	  sd_profile_create_browse_group_list },
	{ SDP_ATTR_VERSION_NUMBER_LIST,
	  sd_profile_create_version_number_list },
	{ SDP_ATTR_SERVICE_DATABASE_STATE,
	  sd_profile_create_service_database_state },
	{ 0, NULL } /* end entry */
};

profile_t	sd_profile_descriptor = {
	SDP_SERVICE_CLASS_SERVICE_DISCOVERY_SERVER,
	0,
	(profile_data_valid_p) NULL,
	(attr_t const * const) &sd_profile_attrs
};