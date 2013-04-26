
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

#include "opt_compat.h"

#ifdef COMPAT_FREEBSD32

/** @file i915_ioc32.c
 * 32-bit ioctl compatibility routines for the i915 DRM.
 */

#include <dev/drm2/drmP.h>
#include <dev/drm2/drm.h>
#include <dev/drm2/i915/i915_drm.h>
#include <dev/drm2/i915/i915_drv.h>
#include <dev/drm2/i915/intel_drv.h>

typedef struct _drm_i915_batchbuffer32 {
	int start;		/* agp offset */
	int used;		/* nr bytes in use */
	int DR1;		/* hw flags for GFX_OP_DRAWRECT_INFO */
	int DR4;		/* window origin for GFX_OP_DRAWRECT_INFO */
	int num_cliprects;	/* mulitpass with multiple cliprects? */
	u32 cliprects;		/* pointer to userspace cliprects */
} drm_i915_batchbuffer32_t;

static int compat_i915_batchbuffer(struct drm_device *dev, void *data, struct drm_file *file_priv)
{
	drm_i915_batchbuffer32_t *batchbuffer32 = data;
	drm_i915_batchbuffer_t batchbuffer;

	batchbuffer.start = batchbuffer32->start;
	batchbuffer.used = batchbuffer32->used;
	batchbuffer.DR1 = batchbuffer32->DR1;
	batchbuffer.DR4 = batchbuffer32->DR4;
	batchbuffer.num_cliprects = batchbuffer32->num_cliprects;
	batchbuffer.cliprects = (void *)(unsigned long)batchbuffer32->cliprects;

	return i915_batchbuffer(dev, (void *)&batchbuffer, file_priv);
}

typedef struct _drm_i915_cmdbuffer32 {
	u32 buf;		/* pointer to userspace command buffer */
	int sz;			/* nr bytes in buf */
	int DR1;		/* hw flags for GFX_OP_DRAWRECT_INFO */
	int DR4;		/* window origin for GFX_OP_DRAWRECT_INFO */
	int num_cliprects;	/* mulitpass with multiple cliprects? */
	u32 cliprects;		/* pointer to userspace cliprects */
} drm_i915_cmdbuffer32_t;

static int compat_i915_cmdbuffer(struct drm_device *dev, void *data, struct drm_file *file_priv)
{
	drm_i915_cmdbuffer32_t *cmdbuffer32 = data;
	drm_i915_cmdbuffer_t cmdbuffer;

	cmdbuffer.sz = cmdbuffer32->sz;
	cmdbuffer.DR1 = cmdbuffer32->DR1;
	cmdbuffer.DR4 = cmdbuffer32->DR4;
	cmdbuffer.num_cliprects = cmdbuffer32->num_cliprects;
	cmdbuffer.cliprects = (void *)(unsigned long)cmdbuffer32->cliprects;

	return i915_cmdbuffer(dev, (void *)&cmdbuffer, file_priv);
}

typedef struct drm_i915_irq_emit32 {
	u32 irq_seq;
} drm_i915_irq_emit32_t;

static int compat_i915_irq_emit(struct drm_device *dev, void *data, struct drm_file *file_priv)
{
	drm_i915_irq_emit32_t *req32 = data;
	drm_i915_irq_emit_t request;

	request.irq_seq = (int *)(unsigned long)req32->irq_seq;

	return i915_irq_emit(dev, (void *)&request, file_priv);
}
typedef struct drm_i915_getparam32 {
	int param;
	u32 value;
} drm_i915_getparam32_t;

static int compat_i915_getparam(struct drm_device *dev, void *data, struct drm_file *file_priv)
{
	drm_i915_getparam32_t *req32 = data;
	drm_i915_getparam_t request;

	request.param = req32->param;
	request.value = (void *)(unsigned long)req32->value;

	return i915_getparam(dev, (void *)&request, file_priv);
}

typedef struct drm_i915_mem_alloc32 {
	int region;
	int alignment;
	int size;
	u32 region_offset;	/* offset from start of fb or agp */
} drm_i915_mem_alloc32_t;

drm_ioctl_desc_t i915_compat_ioctls[] = {
	DRM_IOCTL_DEF(DRM_I915_BATCHBUFFER, compat_i915_batchbuffer, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_I915_CMDBUFFER, compat_i915_cmdbuffer, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_I915_GETPARAM, compat_i915_getparam, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_I915_IRQ_EMIT, compat_i915_irq_emit, DRM_AUTH)
};
int i915_compat_ioctls_nr = DRM_ARRAY_SIZE(i915_compat_ioctls);

#endif