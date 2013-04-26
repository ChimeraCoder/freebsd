
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

#include <dev/drm2/drmP.h>
#include <dev/drm2/drm.h>
#include <dev/drm2/i915/i915_drm.h>
#include <dev/drm2/i915/i915_drv.h>
#include <dev/drm2/i915/intel_drv.h>
#include <dev/drm2/drm_edid.h>
#include <dev/iicbus/iiconf.h>

/**
 * intel_ddc_probe
 *
 */
bool intel_ddc_probe(struct intel_encoder *intel_encoder, int ddc_bus)
{
	struct drm_i915_private *dev_priv = intel_encoder->base.dev->dev_private;
	u8 out_buf[] = { 0x0, 0x0};
	u8 buf[2];
	struct iic_msg msgs[] = {
		{
			.slave = DDC_ADDR << 1,
			.flags = IIC_M_WR,
			.len = 1,
			.buf = out_buf,
		},
		{
			.slave = DDC_ADDR << 1,
			.flags = IIC_M_RD,
			.len = 1,
			.buf = buf,
		}
	};

	return (iicbus_transfer(dev_priv->gmbus[ddc_bus], msgs, 2)
	    == 0/* XXXKIB  2*/);
}

/**
 * intel_ddc_get_modes - get modelist from monitor
 * @connector: DRM connector device to use
 * @adapter: i2c adapter
 *
 * Fetch the EDID information from @connector using the DDC bus.
 */
int
intel_ddc_get_modes(struct drm_connector *connector, device_t adapter)
{
	struct edid *edid;
	int ret = 0;

	edid = drm_get_edid(connector, adapter);
	if (edid) {
		drm_mode_connector_update_edid_property(connector, edid);
		ret = drm_add_edid_modes(connector, edid);
		drm_edid_to_eld(connector, edid);
		connector->display_info.raw_edid = NULL;
		free(edid, DRM_MEM_KMS);
	}

	return ret;
}

static const struct drm_prop_enum_list force_audio_names[] = {
	{ HDMI_AUDIO_OFF_DVI, "force-dvi" },
	{ HDMI_AUDIO_OFF, "off" },
	{ HDMI_AUDIO_AUTO, "auto" },
	{ HDMI_AUDIO_ON, "on" },
};

void
intel_attach_force_audio_property(struct drm_connector *connector)
{
	struct drm_device *dev = connector->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct drm_property *prop;

	prop = dev_priv->force_audio_property;
	if (prop == NULL) {
		prop = drm_property_create_enum(dev, 0,
					   "audio",
					   force_audio_names,
					   DRM_ARRAY_SIZE(force_audio_names));
		if (prop == NULL)
			return;

		dev_priv->force_audio_property = prop;
	}
	drm_connector_attach_property(connector, prop, 0);
}

static const struct drm_prop_enum_list broadcast_rgb_names[] = {
	{ 0, "Full" },
	{ 1, "Limited 16:235" },
};

void
intel_attach_broadcast_rgb_property(struct drm_connector *connector)
{
	struct drm_device *dev = connector->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct drm_property *prop;

	prop = dev_priv->broadcast_rgb_property;
	if (prop == NULL) {
		prop = drm_property_create_enum(dev, DRM_MODE_PROP_ENUM,
		    "Broadcast RGB",
		    broadcast_rgb_names,
		    DRM_ARRAY_SIZE(broadcast_rgb_names));
		if (prop == NULL)
			return;

		dev_priv->broadcast_rgb_property = prop;
	}

	drm_connector_attach_property(connector, prop, 0);
}