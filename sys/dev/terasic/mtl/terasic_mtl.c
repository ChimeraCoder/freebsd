
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

#include <sys/param.h>
#include <sys/bus.h>
#include <sys/condvar.h>
#include <sys/conf.h>
#include <sys/consio.h>				/* struct vt_mode */
#include <sys/fbio.h>				/* video_adapter_t */
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/rman.h>
#include <sys/systm.h>

#include <machine/bus.h>
#include <machine/resource.h>

#include <dev/terasic/mtl/terasic_mtl.h>

/*
 * Device driver for the Terasic Multitouch LCD (MTL).  Three separate
 * sub-drivers that support, respectively, access to device control registers,
 * the pixel frame buffer, and the text frame buffer.  The last of these is
 * also hooked up to syscons.
 *
 * Eventually, the frame buffer control registers and touch screen input FIFO
 * will end up being separate sub-drivers as well.
 *
 * Note: sub-driver detach routines must check whether or not they have
 * attached as they may be called even if the attach routine hasn't been, on
 * an error.
 */

devclass_t	terasic_mtl_devclass;

int
terasic_mtl_attach(struct terasic_mtl_softc *sc)
{
	int error;

	error = terasic_mtl_reg_attach(sc);
	if (error)
		goto error;
	error = terasic_mtl_pixel_attach(sc);
	if (error)
		goto error;
	error = terasic_mtl_text_attach(sc);
	if (error)
		goto error;
	/*
	 * XXXRW: Once we've attached syscons, we can't detach it, so do it
	 * last.
	 */
	error = terasic_mtl_syscons_attach(sc);
	if (error)
		goto error;
	terasic_mtl_blend_default_set(sc, TERASIC_MTL_COLOR_BLACK);
	terasic_mtl_blend_pixel_set(sc, TERASIC_MTL_ALPHA_TRANSPARENT);
	terasic_mtl_blend_textfg_set(sc, TERASIC_MTL_ALPHA_OPAQUE);
	terasic_mtl_blend_textbg_set(sc, TERASIC_MTL_ALPHA_OPAQUE);
	return (0);
error:
	terasic_mtl_text_detach(sc);
	terasic_mtl_pixel_detach(sc);
	terasic_mtl_reg_detach(sc);
	return (error);
}

void
terasic_mtl_detach(struct terasic_mtl_softc *sc)
{

	/* XXXRW: syscons can't detach, but we try anyway, only to panic. */
	terasic_mtl_syscons_detach(sc);

	/* All other aspects of the driver can detach just fine. */
	terasic_mtl_text_detach(sc);
	terasic_mtl_pixel_detach(sc);
	terasic_mtl_reg_detach(sc);
}