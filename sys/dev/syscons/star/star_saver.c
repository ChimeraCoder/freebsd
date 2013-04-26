
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
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/consio.h>
#include <sys/fbio.h>

#include <machine/pc/display.h>

#include <dev/fb/fbreg.h>
#include <dev/fb/splashreg.h>
#include <dev/syscons/syscons.h>

#define NUM_STARS	50

static int blanked;

/*
 * Alternate saver that got its inspiration from a well known utility
 * package for an inferior^H^H^H^H^H^Hfamous OS.
 */
static int
star_saver(video_adapter_t *adp, int blank)
{
	sc_softc_t	*sc;
	scr_stat	*scp;
	int		cell, i;
	static u_char	pattern[] = {"...........++++***   "};
	static char	color16[] = {FG_DARKGREY, FG_LIGHTGREY,
				     FG_WHITE, FG_LIGHTCYAN};
	static char	color8[] = {FG_BLUE, FG_BROWN,
				    FG_LIGHTGREY, FG_CYAN};
	static char	*colors;
	static u_short 	stars[NUM_STARS][2];

	sc = sc_find_softc(adp, NULL);
	if (sc == NULL)
		return EAGAIN;
	scp = sc->cur_scp;

	if (blank) {
		if (adp->va_info.vi_flags & V_INFO_GRAPHICS)
			return EAGAIN;
		if (!blanked) {
			switch (adp->va_mode) {
			case M_PC98_80x25:
			case M_PC98_80x30:
				colors = color8;
				break;
			default:
				colors = color16;
				break;
			}

			/* clear the screen and set the border color */
			sc_vtb_clear(&scp->scr, sc->scr_map[0x20],
				     (FG_LIGHTGREY | BG_BLACK) << 8);
			vidd_set_hw_cursor(adp, -1, -1);
			sc_set_border(scp, 0);
			blanked = TRUE;
			for(i=0; i<NUM_STARS; i++) {
				stars[i][0] =
					random() % (scp->xsize*scp->ysize);
				stars[i][1] = 0;
			}
		}
		cell = random() % NUM_STARS;
		sc_vtb_putc(&scp->scr, stars[cell][0], 
			    sc->scr_map[pattern[stars[cell][1]]],
			    colors[random()%sizeof(color16)] << 8);
		if ((stars[cell][1]+=(random()%4)) >= sizeof(pattern)-1) {
			stars[cell][0] = random() % (scp->xsize*scp->ysize);
			stars[cell][1] = 0;
		}
	} else
		blanked = FALSE;

	return 0;
}

static int
star_init(video_adapter_t *adp)
{
	blanked = FALSE;
	return 0;
}

static int
star_term(video_adapter_t *adp)
{
	return 0;
}

static scrn_saver_t star_module = {
	"star_saver", star_init, star_term, star_saver, NULL,
};

SAVER_MODULE(star_saver, star_module);