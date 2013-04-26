
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
#include <dev/drm2/i915/intel_drv.h>

#define PCI_LBPC 0xf4 /* legacy/combination backlight modes */

void
intel_fixed_panel_mode(struct drm_display_mode *fixed_mode,
		       struct drm_display_mode *adjusted_mode)
{
	adjusted_mode->hdisplay = fixed_mode->hdisplay;
	adjusted_mode->hsync_start = fixed_mode->hsync_start;
	adjusted_mode->hsync_end = fixed_mode->hsync_end;
	adjusted_mode->htotal = fixed_mode->htotal;

	adjusted_mode->vdisplay = fixed_mode->vdisplay;
	adjusted_mode->vsync_start = fixed_mode->vsync_start;
	adjusted_mode->vsync_end = fixed_mode->vsync_end;
	adjusted_mode->vtotal = fixed_mode->vtotal;

	adjusted_mode->clock = fixed_mode->clock;
}

/* adjusted_mode has been preset to be the panel's fixed mode */
void
intel_pch_panel_fitting(struct drm_device *dev,
			int fitting_mode,
			struct drm_display_mode *mode,
			struct drm_display_mode *adjusted_mode)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	int x, y, width, height;

	x = y = width = height = 0;

	/* Native modes don't need fitting */
	if (adjusted_mode->hdisplay == mode->hdisplay &&
	    adjusted_mode->vdisplay == mode->vdisplay)
		goto done;

	switch (fitting_mode) {
	case DRM_MODE_SCALE_CENTER:
		width = mode->hdisplay;
		height = mode->vdisplay;
		x = (adjusted_mode->hdisplay - width + 1)/2;
		y = (adjusted_mode->vdisplay - height + 1)/2;
		break;

	case DRM_MODE_SCALE_ASPECT:
		/* Scale but preserve the aspect ratio */
		{
			u32 scaled_width = adjusted_mode->hdisplay * mode->vdisplay;
			u32 scaled_height = mode->hdisplay * adjusted_mode->vdisplay;
			if (scaled_width > scaled_height) { /* pillar */
				width = scaled_height / mode->vdisplay;
				if (width & 1)
					width++;
				x = (adjusted_mode->hdisplay - width + 1) / 2;
				y = 0;
				height = adjusted_mode->vdisplay;
			} else if (scaled_width < scaled_height) { /* letter */
				height = scaled_width / mode->hdisplay;
				if (height & 1)
					height++;
				y = (adjusted_mode->vdisplay - height + 1) / 2;
				x = 0;
				width = adjusted_mode->hdisplay;
			} else {
				x = y = 0;
				width = adjusted_mode->hdisplay;
				height = adjusted_mode->vdisplay;
			}
		}
		break;

	default:
	case DRM_MODE_SCALE_FULLSCREEN:
		x = y = 0;
		width = adjusted_mode->hdisplay;
		height = adjusted_mode->vdisplay;
		break;
	}

done:
	dev_priv->pch_pf_pos = (x << 16) | y;
	dev_priv->pch_pf_size = (width << 16) | height;
}

static int is_backlight_combination_mode(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;

	if (INTEL_INFO(dev)->gen >= 4)
		return I915_READ(BLC_PWM_CTL2) & BLM_COMBINATION_MODE;

	if (IS_GEN2(dev))
		return I915_READ(BLC_PWM_CTL) & BLM_LEGACY_MODE;

	return 0;
}

static u32 i915_read_blc_pwm_ctl(struct drm_i915_private *dev_priv)
{
	u32 val;

	/* Restore the CTL value if it lost, e.g. GPU reset */

	if (HAS_PCH_SPLIT(dev_priv->dev)) {
		val = I915_READ(BLC_PWM_PCH_CTL2);
		if (dev_priv->saveBLC_PWM_CTL2 == 0) {
			dev_priv->saveBLC_PWM_CTL2 = val;
		} else if (val == 0) {
			I915_WRITE(BLC_PWM_PCH_CTL2,
				   dev_priv->saveBLC_PWM_CTL2);
			val = dev_priv->saveBLC_PWM_CTL2;
		}
	} else {
		val = I915_READ(BLC_PWM_CTL);
		if (dev_priv->saveBLC_PWM_CTL == 0) {
			dev_priv->saveBLC_PWM_CTL = val;
			dev_priv->saveBLC_PWM_CTL2 = I915_READ(BLC_PWM_CTL2);
		} else if (val == 0) {
			I915_WRITE(BLC_PWM_CTL,
				   dev_priv->saveBLC_PWM_CTL);
			I915_WRITE(BLC_PWM_CTL2,
				   dev_priv->saveBLC_PWM_CTL2);
			val = dev_priv->saveBLC_PWM_CTL;
		}
	}

	return val;
}

u32 intel_panel_get_max_backlight(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	u32 max;

	max = i915_read_blc_pwm_ctl(dev_priv);
	if (max == 0) {
		/* XXX add code here to query mode clock or hardware clock
		 * and program max PWM appropriately.
		 */
#if 0
		printf("fixme: max PWM is zero.\n");
#endif
		return 1;
	}

	if (HAS_PCH_SPLIT(dev)) {
		max >>= 16;
	} else {
		if (INTEL_INFO(dev)->gen < 4)
			max >>= 17;
		else
			max >>= 16;

		if (is_backlight_combination_mode(dev))
			max *= 0xff;
	}

	DRM_DEBUG("max backlight PWM = %d\n", max);
	return max;
}

u32 intel_panel_get_backlight(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	u32 val;

	if (HAS_PCH_SPLIT(dev)) {
		val = I915_READ(BLC_PWM_CPU_CTL) & BACKLIGHT_DUTY_CYCLE_MASK;
	} else {
		val = I915_READ(BLC_PWM_CTL) & BACKLIGHT_DUTY_CYCLE_MASK;
		if (INTEL_INFO(dev)->gen < 4)
			val >>= 1;

		if (is_backlight_combination_mode(dev)) {
			u8 lbpc;

			lbpc = pci_read_config(dev->device, PCI_LBPC, 1);
			val *= lbpc;
		}
	}

	DRM_DEBUG("get backlight PWM = %d\n", val);
	return val;
}

static void intel_pch_panel_set_backlight(struct drm_device *dev, u32 level)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	u32 val = I915_READ(BLC_PWM_CPU_CTL) & ~BACKLIGHT_DUTY_CYCLE_MASK;
	I915_WRITE(BLC_PWM_CPU_CTL, val | level);
}

static void intel_panel_actually_set_backlight(struct drm_device *dev, u32 level)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	u32 tmp;

	DRM_DEBUG("set backlight PWM = %d\n", level);

	if (HAS_PCH_SPLIT(dev))
		return intel_pch_panel_set_backlight(dev, level);

	if (is_backlight_combination_mode(dev)) {
		u32 max = intel_panel_get_max_backlight(dev);
		u8 lbpc;

		lbpc = level * 0xfe / max + 1;
		level /= lbpc;
		pci_write_config(dev->device, PCI_LBPC, lbpc, 4);
	}

	tmp = I915_READ(BLC_PWM_CTL);
	if (INTEL_INFO(dev)->gen < 4) 
		level <<= 1;
	tmp &= ~BACKLIGHT_DUTY_CYCLE_MASK;
	I915_WRITE(BLC_PWM_CTL, tmp | level);
}

void intel_panel_set_backlight(struct drm_device *dev, u32 level)
{
	struct drm_i915_private *dev_priv = dev->dev_private;

	dev_priv->backlight_level = level;
	if (dev_priv->backlight_enabled)
		intel_panel_actually_set_backlight(dev, level);
}

void intel_panel_disable_backlight(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;

	dev_priv->backlight_enabled = false;
	intel_panel_actually_set_backlight(dev, 0);
}

void intel_panel_enable_backlight(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;

	if (dev_priv->backlight_level == 0)
		dev_priv->backlight_level = intel_panel_get_max_backlight(dev);

	dev_priv->backlight_enabled = true;
	intel_panel_actually_set_backlight(dev, dev_priv->backlight_level);
}

static void intel_panel_init_backlight(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;

	dev_priv->backlight_level = intel_panel_get_backlight(dev);
	dev_priv->backlight_enabled = dev_priv->backlight_level != 0;
}

enum drm_connector_status
intel_panel_detect(struct drm_device *dev)
{
#if 0
	struct drm_i915_private *dev_priv = dev->dev_private;
#endif

	if (i915_panel_ignore_lid)
		return i915_panel_ignore_lid > 0 ?
			connector_status_connected :
			connector_status_disconnected;

	/* opregion lid state on HP 2540p is wrong at boot up,
	 * appears to be either the BIOS or Linux ACPI fault */
#if 0
	/* Assume that the BIOS does not lie through the OpRegion... */
	if (dev_priv->opregion.lid_state)
		return ioread32(dev_priv->opregion.lid_state) & 0x1 ?
			connector_status_connected :
			connector_status_disconnected;
#endif

	return connector_status_unknown;
}

int intel_panel_setup_backlight(struct drm_device *dev)
{
	intel_panel_init_backlight(dev);
	return 0;
}

void intel_panel_destroy_backlight(struct drm_device *dev)
{
	return;
}