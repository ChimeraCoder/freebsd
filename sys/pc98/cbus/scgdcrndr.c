
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

#include "opt_syscons.h"
#include "opt_gdc.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/fbio.h>
#include <sys/consio.h>

#include <dev/fb/fbreg.h>
#include <dev/syscons/syscons.h>

#ifndef SC_RENDER_DEBUG
#define SC_RENDER_DEBUG		0
#endif

static vr_clear_t		gdc_txtclear;
static vr_draw_border_t		gdc_txtborder;
static vr_draw_t		gdc_txtdraw;
static vr_set_cursor_t		gdc_txtcursor_shape;
static vr_draw_cursor_t		gdc_txtcursor;
#ifndef SC_NO_CUTPASTE
static vr_draw_mouse_t		gdc_txtmouse;
#else
#define gdc_txtmouse		(vr_draw_mouse_t *)gdc_nop
#endif

#ifndef SC_NO_MODE_CHANGE
static vr_draw_border_t		gdc_grborder;
#endif

static void			gdc_nop(scr_stat *scp, ...);

static sc_rndr_sw_t txtrndrsw = {
	(vr_init_t *)gdc_nop,
	gdc_txtclear,
	gdc_txtborder,
	gdc_txtdraw,	
	gdc_txtcursor_shape,
	gdc_txtcursor,
	(vr_blink_cursor_t *)gdc_nop,
	(vr_set_mouse_t *)gdc_nop,
	gdc_txtmouse,
};
RENDERER(gdc, 0, txtrndrsw, gdc_set);

#ifndef SC_NO_MODE_CHANGE
static sc_rndr_sw_t grrndrsw = {
	(vr_init_t *)gdc_nop,
	(vr_clear_t *)gdc_nop,
	gdc_grborder,
	(vr_draw_t *)gdc_nop,
	(vr_set_cursor_t *)gdc_nop,
	(vr_draw_cursor_t *)gdc_nop,
	(vr_blink_cursor_t *)gdc_nop,
	(vr_set_mouse_t *)gdc_nop,
	(vr_draw_mouse_t *)gdc_nop,
};
RENDERER(gdc, GRAPHICS_MODE, grrndrsw, gdc_set);
#endif /* SC_NO_MODE_CHANGE */

RENDERER_MODULE(gdc, gdc_set);

static void
gdc_nop(scr_stat *scp, ...)
{
}

/* text mode renderer */

static void
gdc_txtclear(scr_stat *scp, int c, int attr)
{
	sc_vtb_clear(&scp->scr, c, attr);
}

static void
gdc_txtborder(scr_stat *scp, int color)
{
	vidd_set_border(scp->sc->adp, color);
}

static void
gdc_txtdraw(scr_stat *scp, int from, int count, int flip)
{
	vm_offset_t p;
	int c;
	int a;

	if (from + count > scp->xsize*scp->ysize)
		count = scp->xsize*scp->ysize - from;

	if (flip) {
		for (p = sc_vtb_pointer(&scp->scr, from); count-- > 0; ++from) {
			c = sc_vtb_getc(&scp->vtb, from);
			a = sc_vtb_geta(&scp->vtb, from);
#if 0
			a ^= 0x0800;
#else
			a = (a & 0x8800) | ((a & 0x7000) >> 4) 
				| ((a & 0x0700) << 4);
#endif
			p = sc_vtb_putchar(&scp->scr, p, c, a);
		}
	} else {
		sc_vtb_copy(&scp->vtb, from, &scp->scr, from, count);
	}
}

static void
gdc_txtcursor_shape(scr_stat *scp, int base, int height, int blink)
{
	if (base < 0 || base >= scp->font_size)
		return;
	/* the caller may set height <= 0 in order to disable the cursor */
	vidd_set_hw_cursor_shape(scp->sc->adp, base, height, scp->font_size,
	    blink);
}

static void
gdc_txtcursor(scr_stat *scp, int at, int blink, int on, int flip)
{
	if (on) {
		scp->status |= VR_CURSOR_ON;
		vidd_set_hw_cursor(scp->sc->adp, at%scp->xsize,
		    at/scp->xsize);
	} else {
		if (scp->status & VR_CURSOR_ON)
			vidd_set_hw_cursor(scp->sc->adp, -1, -1);
		scp->status &= ~VR_CURSOR_ON;
	}
}

#ifndef SC_NO_CUTPASTE

static void
draw_txtmouse(scr_stat *scp, int x, int y)
{
	int at;
	int a;

	at = (y/scp->font_size - scp->yoff)*scp->xsize + x/8 - scp->xoff;
	a = sc_vtb_geta(&scp->vtb, at);
#if 0
	a ^= 0x0800;
#else
	a = (a & 0x8800) | ((a & 0x7000) >> 4) 
		| ((a & 0x0700) << 4);
#endif
	sc_vtb_putc(&scp->scr, at, sc_vtb_getc(&scp->scr, at), a);
}

static void
remove_txtmouse(scr_stat *scp, int x, int y)
{
}

static void 
gdc_txtmouse(scr_stat *scp, int x, int y, int on)
{
	if (on)
		draw_txtmouse(scp, x, y);
	else
		remove_txtmouse(scp, x, y);
}

#endif /* SC_NO_CUTPASTE */

#ifndef SC_NO_MODE_CHANGE

/* graphics mode renderer */

static void
gdc_grborder(scr_stat *scp, int color)
{
	vidd_set_border(scp->sc->adp, color);
}

#endif /* SC_NO_MODE_CHANGE */