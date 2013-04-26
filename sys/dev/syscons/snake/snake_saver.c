
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
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/sysctl.h>
#include <sys/consio.h>
#include <sys/fbio.h>
#include <sys/resourcevar.h>
#include <sys/smp.h>

#include <machine/pc/display.h>

#include <dev/fb/fbreg.h>
#include <dev/fb/splashreg.h>
#include <dev/syscons/syscons.h>

static u_char	*message;
static int	*messagep;
static int	messagelen;
static int	blanked;

#define MSGBUF_LEN 	70

static int	nofancy = 0;
TUNABLE_INT("hw.syscons.saver_snake_nofancy", &nofancy);

#define FANCY_SNAKE 	(!nofancy)
#define LOAD_HIGH(ld) 	(((ld * 100 + FSCALE / 2) >> FSHIFT) / 100)
#define LOAD_LOW(ld) 	(((ld * 100 + FSCALE / 2) >> FSHIFT) % 100)

static inline void update_msg(void);

static int
snake_saver(video_adapter_t *adp, int blank)
{
	static int	dirx, diry;
	int		f, color, load;
	sc_softc_t	*sc;
	scr_stat	*scp;

/* XXX hack for minimal changes. */
#define	save	message
#define	savs	messagep

	sc = sc_find_softc(adp, NULL);
	if (sc == NULL)
		return EAGAIN;
	scp = sc->cur_scp;

	if (blank) {
		if (adp->va_info.vi_flags & V_INFO_GRAPHICS)
			return EAGAIN;
		if (blanked <= 0) {
			sc_vtb_clear(&scp->scr, sc->scr_map[0x20],
				     (FG_LIGHTGREY | BG_BLACK) << 8);
			vidd_set_hw_cursor(adp, -1, -1);
			sc_set_border(scp, 0);
			dirx = (scp->xpos ? 1 : -1);
			diry = (scp->ypos ?
				scp->xsize : -scp->xsize);
			for (f=0; f< messagelen; f++)
				savs[f] = scp->xpos + scp->ypos*scp->xsize;
			sc_vtb_putc(&scp->scr, savs[0], sc->scr_map[*save],
				    (FG_LIGHTGREY | BG_BLACK) << 8);
			blanked = 1;
		}
		if (blanked++ < 4)
			return 0;
		blanked = 1;
		sc_vtb_putc(&scp->scr, savs[messagelen - 1], sc->scr_map[0x20],
			    (FG_LIGHTGREY | BG_BLACK) << 8);
		for (f=messagelen-1; f > 0; f--)
			savs[f] = savs[f-1];
		f = savs[0];
		if ((f % scp->xsize) == 0 ||
		    (f % scp->xsize) == scp->xsize - 1 ||
		    (random() % 50) == 0)
			dirx = -dirx;
		if ((f / scp->xsize) == 0 ||
		    (f / scp->xsize) == scp->ysize - 1 ||
		    (random() % 20) == 0)
			diry = -diry;
		savs[0] += dirx + diry;
		if (FANCY_SNAKE) {
			update_msg();
			load = ((averunnable.ldavg[0] * 100 + FSCALE / 2) >> FSHIFT);
			if (load == 0)
				color = FG_LIGHTGREY | BG_BLACK;
			else if (load / mp_ncpus <= 50)
				color = FG_LIGHTGREEN | BG_BLACK;
			else if (load / mp_ncpus <= 75)
				color = FG_YELLOW | BG_BLACK;
			else if (load / mp_ncpus <= 99)
				color = FG_LIGHTRED | BG_BLACK;
			else
				color = FG_RED | FG_BLINK | BG_BLACK;
		} else
			color = FG_LIGHTGREY | BG_BLACK;

		for (f=messagelen-1; f>=0; f--)
			sc_vtb_putc(&scp->scr, savs[f], sc->scr_map[save[f]],
				    color << 8);
	} else
		blanked = 0;

	return 0;
}

static inline void
update_msg(void)
{
	if (!FANCY_SNAKE) {
		messagelen = sprintf(message, "%s %s", ostype, osrelease);
		return;
	}
	messagelen = snprintf(message, MSGBUF_LEN,
	    "%s %s (%d.%02d %d.%02d, %d.%02d)",
	    ostype, osrelease,
	    LOAD_HIGH(averunnable.ldavg[0]), LOAD_LOW(averunnable.ldavg[0]),
	    LOAD_HIGH(averunnable.ldavg[1]), LOAD_LOW(averunnable.ldavg[1]),
	    LOAD_HIGH(averunnable.ldavg[2]), LOAD_LOW(averunnable.ldavg[2]));
}

static int
snake_init(video_adapter_t *adp)
{
	message = malloc(MSGBUF_LEN, M_DEVBUF, M_WAITOK);
	messagep = malloc(MSGBUF_LEN * sizeof *messagep, M_DEVBUF, M_WAITOK);
	update_msg();
	return 0;
}

static int
snake_term(video_adapter_t *adp)
{
	free(message, M_DEVBUF);
	free(messagep, M_DEVBUF);
	return 0;
}

static scrn_saver_t snake_module = {
	"snake_saver", snake_init, snake_term, snake_saver, NULL,
};

SAVER_MODULE(snake_saver, snake_module);