
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

#include "dev/drm/drmP.h"
#include "dev/drm/drm.h"
#include "dev/drm/mga_drm.h"
#include "dev/drm/mga_drv.h"
#include "dev/drm/mga_ucode.h"

#define MGA_WARP_CODE_ALIGN		256	/* in bytes */

#define WARP_UCODE_SIZE( which )					\
	((sizeof(which) / MGA_WARP_CODE_ALIGN + 1) * MGA_WARP_CODE_ALIGN)

#define WARP_UCODE_INSTALL( which, where )				\
do {									\
	DRM_DEBUG( " pcbase = 0x%08lx  vcbase = %p\n", pcbase, vcbase );\
	dev_priv->warp_pipe_phys[where] = pcbase;			\
	memcpy( vcbase, which, sizeof(which) );				\
	pcbase += WARP_UCODE_SIZE( which );				\
	vcbase += WARP_UCODE_SIZE( which );				\
} while (0)

static const unsigned int mga_warp_g400_microcode_size =
	       (WARP_UCODE_SIZE(warp_g400_tgz) +
		WARP_UCODE_SIZE(warp_g400_tgza) +
		WARP_UCODE_SIZE(warp_g400_tgzaf) +
		WARP_UCODE_SIZE(warp_g400_tgzf) +
		WARP_UCODE_SIZE(warp_g400_tgzs) +
		WARP_UCODE_SIZE(warp_g400_tgzsa) +
		WARP_UCODE_SIZE(warp_g400_tgzsaf) +
		WARP_UCODE_SIZE(warp_g400_tgzsf) +
		WARP_UCODE_SIZE(warp_g400_t2gz) +
		WARP_UCODE_SIZE(warp_g400_t2gza) +
		WARP_UCODE_SIZE(warp_g400_t2gzaf) +
		WARP_UCODE_SIZE(warp_g400_t2gzf) +
		WARP_UCODE_SIZE(warp_g400_t2gzs) +
		WARP_UCODE_SIZE(warp_g400_t2gzsa) +
		WARP_UCODE_SIZE(warp_g400_t2gzsaf) +
		WARP_UCODE_SIZE(warp_g400_t2gzsf));

static const unsigned int mga_warp_g200_microcode_size =
	       (WARP_UCODE_SIZE(warp_g200_tgz) +
		WARP_UCODE_SIZE(warp_g200_tgza) +
		WARP_UCODE_SIZE(warp_g200_tgzaf) +
		WARP_UCODE_SIZE(warp_g200_tgzf) +
		WARP_UCODE_SIZE(warp_g200_tgzs) +
		WARP_UCODE_SIZE(warp_g200_tgzsa) +
		WARP_UCODE_SIZE(warp_g200_tgzsaf) +
		WARP_UCODE_SIZE(warp_g200_tgzsf));


unsigned int mga_warp_microcode_size(const drm_mga_private_t * dev_priv)
{
	switch (dev_priv->chipset) {
	case MGA_CARD_TYPE_G400:
	case MGA_CARD_TYPE_G550:
		return PAGE_ALIGN(mga_warp_g400_microcode_size);
	case MGA_CARD_TYPE_G200:
		return PAGE_ALIGN(mga_warp_g200_microcode_size);
	default:
		DRM_ERROR("Unknown chipset value: 0x%x\n", dev_priv->chipset);
		return 0;
	}
}

static int mga_warp_install_g400_microcode(drm_mga_private_t * dev_priv)
{
	unsigned char *vcbase = dev_priv->warp->virtual;
	unsigned long pcbase = dev_priv->warp->offset;

	memset(dev_priv->warp_pipe_phys, 0, sizeof(dev_priv->warp_pipe_phys));

	WARP_UCODE_INSTALL(warp_g400_tgz, MGA_WARP_TGZ);
	WARP_UCODE_INSTALL(warp_g400_tgzf, MGA_WARP_TGZF);
	WARP_UCODE_INSTALL(warp_g400_tgza, MGA_WARP_TGZA);
	WARP_UCODE_INSTALL(warp_g400_tgzaf, MGA_WARP_TGZAF);
	WARP_UCODE_INSTALL(warp_g400_tgzs, MGA_WARP_TGZS);
	WARP_UCODE_INSTALL(warp_g400_tgzsf, MGA_WARP_TGZSF);
	WARP_UCODE_INSTALL(warp_g400_tgzsa, MGA_WARP_TGZSA);
	WARP_UCODE_INSTALL(warp_g400_tgzsaf, MGA_WARP_TGZSAF);

	WARP_UCODE_INSTALL(warp_g400_t2gz, MGA_WARP_T2GZ);
	WARP_UCODE_INSTALL(warp_g400_t2gzf, MGA_WARP_T2GZF);
	WARP_UCODE_INSTALL(warp_g400_t2gza, MGA_WARP_T2GZA);
	WARP_UCODE_INSTALL(warp_g400_t2gzaf, MGA_WARP_T2GZAF);
	WARP_UCODE_INSTALL(warp_g400_t2gzs, MGA_WARP_T2GZS);
	WARP_UCODE_INSTALL(warp_g400_t2gzsf, MGA_WARP_T2GZSF);
	WARP_UCODE_INSTALL(warp_g400_t2gzsa, MGA_WARP_T2GZSA);
	WARP_UCODE_INSTALL(warp_g400_t2gzsaf, MGA_WARP_T2GZSAF);

	return 0;
}

static int mga_warp_install_g200_microcode(drm_mga_private_t * dev_priv)
{
	unsigned char *vcbase = dev_priv->warp->virtual;
	unsigned long pcbase = dev_priv->warp->offset;

	memset(dev_priv->warp_pipe_phys, 0, sizeof(dev_priv->warp_pipe_phys));

	WARP_UCODE_INSTALL(warp_g200_tgz, MGA_WARP_TGZ);
	WARP_UCODE_INSTALL(warp_g200_tgzf, MGA_WARP_TGZF);
	WARP_UCODE_INSTALL(warp_g200_tgza, MGA_WARP_TGZA);
	WARP_UCODE_INSTALL(warp_g200_tgzaf, MGA_WARP_TGZAF);
	WARP_UCODE_INSTALL(warp_g200_tgzs, MGA_WARP_TGZS);
	WARP_UCODE_INSTALL(warp_g200_tgzsf, MGA_WARP_TGZSF);
	WARP_UCODE_INSTALL(warp_g200_tgzsa, MGA_WARP_TGZSA);
	WARP_UCODE_INSTALL(warp_g200_tgzsaf, MGA_WARP_TGZSAF);

	return 0;
}

int mga_warp_install_microcode(drm_mga_private_t * dev_priv)
{
	const unsigned int size = mga_warp_microcode_size(dev_priv);

	DRM_DEBUG("MGA ucode size = %d bytes\n", size);
	if (size > dev_priv->warp->size) {
		DRM_ERROR("microcode too large! (%u > %lu)\n",
			  size, dev_priv->warp->size);
		return -ENOMEM;
	}

	switch (dev_priv->chipset) {
	case MGA_CARD_TYPE_G400:
	case MGA_CARD_TYPE_G550:
		return mga_warp_install_g400_microcode(dev_priv);
	case MGA_CARD_TYPE_G200:
		return mga_warp_install_g200_microcode(dev_priv);
	default:
		return -EINVAL;
	}
}

#define WMISC_EXPECTED		(MGA_WUCODECACHE_ENABLE | MGA_WMASTER_ENABLE)

int mga_warp_init(drm_mga_private_t * dev_priv)
{
	u32 wmisc;

	/* FIXME: Get rid of these damned magic numbers...
	 */
	switch (dev_priv->chipset) {
	case MGA_CARD_TYPE_G400:
	case MGA_CARD_TYPE_G550:
		MGA_WRITE(MGA_WIADDR2, MGA_WMODE_SUSPEND);
		MGA_WRITE(MGA_WGETMSB, 0x00000E00);
		MGA_WRITE(MGA_WVRTXSZ, 0x00001807);
		MGA_WRITE(MGA_WACCEPTSEQ, 0x18000000);
		break;
	case MGA_CARD_TYPE_G200:
		MGA_WRITE(MGA_WIADDR, MGA_WMODE_SUSPEND);
		MGA_WRITE(MGA_WGETMSB, 0x1606);
		MGA_WRITE(MGA_WVRTXSZ, 7);
		break;
	default:
		return -EINVAL;
	}

	MGA_WRITE(MGA_WMISC, (MGA_WUCODECACHE_ENABLE |
			      MGA_WMASTER_ENABLE | MGA_WCACHEFLUSH_ENABLE));
	wmisc = MGA_READ(MGA_WMISC);
	if (wmisc != WMISC_EXPECTED) {
		DRM_ERROR("WARP engine config failed! 0x%x != 0x%x\n",
			  wmisc, WMISC_EXPECTED);
		return -EINVAL;
	}

	return 0;
}