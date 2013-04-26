
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/consio.h>
#include <sys/fbio.h>

#include <machine/pc/display.h>

#include <dev/fb/fbreg.h>
#include <dev/fb/splashreg.h>
#include <dev/syscons/syscons.h>

static int splash_on = FALSE;

static int txt_init(video_adapter_t *adp);
static int txt_end(video_adapter_t *adp);
static int txt_splash(video_adapter_t * adp, const int on);

/* These are rows by columns of the text-mode display device. */
#define BIN_IMAGE_WIDTH		80
#define BIN_IMAGE_HEIGHT	25

static splash_decoder_t txt_decoder = {
       .name = "splash_txt",
       .init = txt_init,
       .term = txt_end,
       .splash = txt_splash,
       .data_type = SPLASH_IMAGE,
};

SPLASH_DECODER(splash_txt, txt_decoder);

static void
draw_text_splash(sc_softc_t *sc)
{
	u_int x, y;
	u_char ch, attr;
	u_char *pdata = txt_decoder.data;

	/* Init failed. */
	if (txt_decoder.data == NULL)
		return;
	for (y = 0; y < BIN_IMAGE_HEIGHT; y++) {
		for (x = 0; x < BIN_IMAGE_WIDTH; x++) {
			ch = *pdata++;
			attr = *pdata++;
			sc_vtb_putc(&sc->cur_scp->scr,
			    (y * sc->cur_scp->xsize) + x,
			    sc->scr_map[ch], (int)attr << 8);
		}
	}
}

static int
txt_init(video_adapter_t *adp)
{

	/* Ensure that the image data exists. */
	if (txt_decoder.data == NULL || txt_decoder.data_size <= 0) {
		printf("splash_txt: No ASCII bitmap file found\n");
		return (ENODEV);
	}
	return (0);
}

static int
txt_end(video_adapter_t *adp)
{

	return (0);
}

static int
txt_splash(video_adapter_t *adp, const int on)
{
	sc_softc_t *sc;
	scr_stat *scp;

	sc = sc_find_softc(adp, NULL);
	if (sc == NULL)
		return (EAGAIN);
	scp = sc->cur_scp;
	if (on) {
		if (!splash_on) {
			if (adp->va_info.vi_flags & V_INFO_GRAPHICS)
				return EAGAIN;
			/* Clear screen and set border colour. */
			sc_vtb_clear(&scp->scr, sc->scr_map[0x20],
			    (FG_LIGHTGREY | BG_BLACK) << 8);
			(*vidsw[adp->va_index]->set_hw_cursor)(adp, -1, -1);
			sc_set_border(scp, 0);
			splash_on = TRUE;
			/* Display the splash screen. */
			draw_text_splash(sc);
		}
		return (0);
	} else {
		/* The video mode will be restored by the caller. */
		splash_on = FALSE;
		return (0);
	}
}