
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

static int
green_saver(video_adapter_t *adp, int blank)
{
	vidd_blank_display(adp,
	    (blank) ? V_DISPLAY_STAND_BY : V_DISPLAY_ON);
	return 0;
}

static int
green_init(video_adapter_t *adp)
{
	if (vidd_blank_display(adp, V_DISPLAY_ON) == 0)
		return 0;
	return ENODEV;
}

static int
green_term(video_adapter_t *adp)
{
	return 0;
}

static scrn_saver_t green_module = {
	"green_saver", green_init, green_term, green_saver, NULL,
};

SAVER_MODULE(green_saver, green_module);