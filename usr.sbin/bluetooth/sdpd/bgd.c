
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

#include <bluetooth.h>
#include <sdp.h>
#include <string.h>
#include "profile.h"

static int32_t
bgd_profile_create_service_class_id_list(
		uint8_t *buf, uint8_t const * const eob,
		uint8_t const *data, uint32_t datalen)
{
	static uint16_t	service_classes[] = {
		SDP_SERVICE_CLASS_BROWSE_GROUP_DESCRIPTOR
	};

	return (common_profile_create_service_class_id_list(
			buf, eob,
			(uint8_t const *) service_classes,
			sizeof(service_classes)));
}

static int32_t
bgd_profile_create_service_name(
		uint8_t *buf, uint8_t const * const eob,
		uint8_t const *data, uint32_t datalen)
{
	static char	service_name[] = "Public Browse Group Root";

	return (common_profile_create_string8(
			buf, eob,
			(uint8_t const *) service_name, strlen(service_name)));
}

static int32_t
bgd_profile_create_group_id(
		uint8_t *buf, uint8_t const * const eob,
		uint8_t const *data, uint32_t datalen)
{
	if (buf + 3 > eob)
		return (-1);

	SDP_PUT8(SDP_DATA_UUID16, buf);
	SDP_PUT16(SDP_SERVICE_CLASS_PUBLIC_BROWSE_GROUP, buf);

	return (3);
}

static attr_t	bgd_profile_attrs[] = {
	{ SDP_ATTR_SERVICE_RECORD_HANDLE,
	  common_profile_create_service_record_handle },
	{ SDP_ATTR_SERVICE_CLASS_ID_LIST,
	  bgd_profile_create_service_class_id_list },
	{ SDP_ATTR_LANGUAGE_BASE_ATTRIBUTE_ID_LIST,
	  common_profile_create_language_base_attribute_id_list },
	{ SDP_ATTR_PRIMARY_LANGUAGE_BASE_ID + SDP_ATTR_SERVICE_NAME_OFFSET, 
	  bgd_profile_create_service_name },
	{ SDP_ATTR_PRIMARY_LANGUAGE_BASE_ID + SDP_ATTR_SERVICE_DESCRIPTION_OFFSET, 
	  bgd_profile_create_service_name },
	{ SDP_ATTR_PRIMARY_LANGUAGE_BASE_ID + SDP_ATTR_PROVIDER_NAME_OFFSET,
	  common_profile_create_service_provider_name },
	{ SDP_ATTR_GROUP_ID,
	  bgd_profile_create_group_id },
	{ 0, NULL } /* end entry */
};

profile_t	bgd_profile_descriptor = {
	SDP_SERVICE_CLASS_BROWSE_GROUP_DESCRIPTOR,
	0,
	(profile_data_valid_p) NULL,
	(attr_t const * const) &bgd_profile_attrs
};