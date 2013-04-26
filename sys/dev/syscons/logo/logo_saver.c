
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
#include <sys/syslog.h>
#include <sys/consio.h>
#include <sys/fbio.h>

#include <dev/fb/fbreg.h>
#include <dev/fb/splashreg.h>
#include <dev/syscons/syscons.h>

#define SAVER_NAME	 "logo_saver"

#define SET_ORIGIN(adp, o) do {				\
	int oo = o;					\
	if (oo != last_origin)				\
	    vidd_set_win_org(adp, last_origin = oo);	\
	} while (0)

extern unsigned int	 logo_w;
extern unsigned int	 logo_h;
extern unsigned char	 logo_pal[];
extern unsigned char	 logo_img[];
extern unsigned int	 logo_img_size;

static u_char		*vid;
static int		 banksize, scrmode, bpsl, scrw, scrh;
static int		 blanked;

static void
logo_blit(video_adapter_t *adp, int x, int y)
{
	int d, l, o, p;
	int last_origin = -1;
	
	for (o = 0, p = y * bpsl + x; p > banksize; p -= banksize)
		o += banksize;
	SET_ORIGIN(adp, o);
	
	for (d = 0; d < logo_img_size; d += logo_w) {
		if (p + logo_w < banksize) {
			bcopy(logo_img + d, vid + p, logo_w);
			p += bpsl;
		} else if (p < banksize) {
			l = banksize - p;
			bcopy(logo_img + d, vid + p, l);
			SET_ORIGIN(adp, (o += banksize));
			bcopy(logo_img + d + l, vid, logo_w - l);
			p += bpsl - banksize;
		} else {
			p -= banksize;
			SET_ORIGIN(adp, (o += banksize));
			bcopy(logo_img + d, vid + p, logo_w);
			p += bpsl;
		}
	}
}

static void
logo_update(video_adapter_t *adp)
{
	static int xpos = 0, ypos = 0;
	static int xinc = 1, yinc = 1;
	
	/* Turn when you hit the edge */
	if ((xpos + logo_w + xinc > scrw) || (xpos + xinc < 0))
		xinc = -xinc;
	if ((ypos + logo_h + yinc > scrh) || (ypos + yinc < 0))
		yinc = -yinc;
	xpos += xinc;
	ypos += yinc;
	
	/* XXX Relies on margin around logo to erase trail */
	logo_blit(adp, xpos, ypos);
}

static int
logo_saver(video_adapter_t *adp, int blank)
{
	int pl;
	
	if (blank) {
		/* switch to graphics mode */
		if (blanked <= 0) {
			pl = splhigh();
			vidd_set_mode(adp, scrmode);
			vidd_load_palette(adp, logo_pal);
			vidd_set_border(adp, 0);
			blanked++;
			vid = (u_char *)adp->va_window;
			banksize = adp->va_window_size;
			bpsl = adp->va_line_width;
			splx(pl);
			vidd_clear(adp);
		}
		logo_update(adp);
	} else {
		blanked = 0;
	}
	return (0);
}

static int
logo_init(video_adapter_t *adp)
{
	video_info_t info;
	
	if (!vidd_get_info(adp, M_VESA_CG800x600, &info)) {
		scrmode = M_VESA_CG800x600;
	} else if (!vidd_get_info(adp, M_VGA_CG320, &info)) {
		scrmode = M_VGA_CG320;
	} else if (!vidd_get_info(adp, M_PC98_PEGC640x480, &info)) {
		scrmode = M_PC98_PEGC640x480;
	} else if (!vidd_get_info(adp, M_PC98_PEGC640x400, &info)) {
		scrmode = M_PC98_PEGC640x400;
	} else {
		log(LOG_NOTICE,
		    "%s: the console does not support M_VGA_CG320\n",
		    SAVER_NAME);
		return (ENODEV);
	}
	
	scrw = info.vi_width;
	scrh = info.vi_height;
	
	return (0);
}

static int
logo_term(video_adapter_t *adp)
{
	return (0);
}

static scrn_saver_t logo_module = {
	SAVER_NAME,
	logo_init,
	logo_term,
	logo_saver,
	NULL
};

#ifdef BEASTIE_LOGO
SAVER_MODULE(beastie_saver, logo_module);
#else
SAVER_MODULE(logo_saver, logo_module);
#endif