
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

#include <dev/fb/fbreg.h>
#include <dev/fb/splashreg.h>
#include <dev/syscons/syscons.h>

static u_char palette[256*3];

static int
fade_saver(video_adapter_t *adp, int blank)
{
	static int count = 0;
	u_char pal[256*3];
	int i;

	if (blank) {
		if (ISPALAVAIL(adp->va_flags)) {
			if (count <= 0)
				vidd_save_palette(adp, palette);
			if (count < 256) {
				pal[0] = pal[1] = pal[2] = 0;
				for (i = 3; i < 256*3; i++) {
					if (palette[i] - count > 60)
						pal[i] = palette[i] - count;
					else
						pal[i] = 60;
				}
				vidd_load_palette(adp, pal);
				count++;
			}
		} else {
	    		vidd_blank_display(adp, V_DISPLAY_BLANK);
		}
	} else {
		if (ISPALAVAIL(adp->va_flags)) {
			vidd_load_palette(adp, palette);
			count = 0;
		} else {
	    		vidd_blank_display(adp, V_DISPLAY_ON);
		}
	}
	return 0;
}

static int
fade_init(video_adapter_t *adp)
{
	if (!ISPALAVAIL(adp->va_flags) &&
	    vidd_blank_display(adp, V_DISPLAY_ON) != 0)
		return ENODEV;
	return 0;
}

static int
fade_term(video_adapter_t *adp)
{
	return 0;
}

static scrn_saver_t fade_module = {
	"fade_saver", fade_init, fade_term, fade_saver, NULL,
};

SAVER_MODULE(fade_saver, fade_module);