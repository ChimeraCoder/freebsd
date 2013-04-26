
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

#include "opt_vga.h"

#ifndef VGA_NO_MODE_CHANGE

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/tty.h>
#include <sys/kernel.h>
#include <sys/fbio.h>
#include <sys/consio.h>

#include <dev/fb/vesa.h>

#include <dev/fb/fbreg.h>
#include <dev/syscons/syscons.h>

static tsw_ioctl_t *prev_user_ioctl;

static int
vesa_ioctl(struct tty *tp, u_long cmd, caddr_t data, struct thread *td)
{
	scr_stat *scp;
	int mode;

	scp = SC_STAT(tp);

	switch (cmd) {

	/* generic text modes */
	case SW_TEXT_132x25: case SW_TEXT_132x30:
	case SW_TEXT_132x43: case SW_TEXT_132x50:
	case SW_TEXT_132x60:
		if (!(scp->sc->adp->va_flags & V_ADP_MODECHANGE))
			return ENODEV;
		return sc_set_text_mode(scp, tp, cmd & 0xff, 0, 0, 0, 0);

	/* text modes */
	case SW_VESA_C80x60:
	case SW_VESA_C132x25:
	case SW_VESA_C132x43:
	case SW_VESA_C132x50:
	case SW_VESA_C132x60:
		if (!(scp->sc->adp->va_flags & V_ADP_MODECHANGE))
			return ENODEV;
		mode = (cmd & 0xff) + M_VESA_BASE;
		return sc_set_text_mode(scp, tp, mode, 0, 0, 0, 0);

	/* graphics modes */
	case SW_VESA_32K_320: 	case SW_VESA_64K_320: 
	case SW_VESA_FULL_320:

	case SW_VESA_CG640x400:

	case SW_VESA_CG640x480:
	case SW_VESA_32K_640:	case SW_VESA_64K_640:
	case SW_VESA_FULL_640:

	case SW_VESA_800x600:	case SW_VESA_CG800x600:
	case SW_VESA_32K_800:	case SW_VESA_64K_800:
	case SW_VESA_FULL_800:

	case SW_VESA_1024x768:	case SW_VESA_CG1024x768:
	case SW_VESA_32K_1024:	case SW_VESA_64K_1024:
	case SW_VESA_FULL_1024:

	case SW_VESA_1280x1024:	case SW_VESA_CG1280x1024:
	case SW_VESA_32K_1280:	case SW_VESA_64K_1280:
	case SW_VESA_FULL_1280:
		if (!(scp->sc->adp->va_flags & V_ADP_MODECHANGE))
			return ENODEV;
		mode = (cmd & 0xff) + M_VESA_BASE;
		return sc_set_graphics_mode(scp, tp, mode);
	default:
		if (IOCGROUP(cmd) == 'V') {
			if (!(scp->sc->adp->va_flags & V_ADP_MODECHANGE))
				return ENODEV;

			mode = (cmd & 0xff) + M_VESA_BASE;

			if (((cmd & IOC_DIRMASK) == IOC_VOID) &&
			    (mode > M_VESA_FULL_1280) &&
			    (mode < M_VESA_MODE_MAX))
				return sc_set_graphics_mode(scp, tp, mode);
		}
	}

	if (prev_user_ioctl)
		return (*prev_user_ioctl)(tp, cmd, data, td);
	else
		return ENOIOCTL;
}

int
vesa_load_ioctl(void)
{
	if (prev_user_ioctl)
		return EBUSY;
	prev_user_ioctl = sc_user_ioctl;
	sc_user_ioctl = vesa_ioctl;
	return 0;
}

int
vesa_unload_ioctl(void)
{
	if (sc_user_ioctl != vesa_ioctl)
		return EBUSY;
	sc_user_ioctl = prev_user_ioctl;
	prev_user_ioctl = NULL;
	return 0;
}

#endif /* VGA_NO_MODE_CHANGE */