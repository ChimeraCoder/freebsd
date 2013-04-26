
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
#include <sys/condvar.h>
#include <sys/kernel.h>
#include <sys/kthread.h>
#include <sys/lock.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/consio.h>
#include <sys/fbio.h>

#include <dev/fb/fbreg.h>
#include <dev/fb/splashreg.h>
#include <dev/syscons/syscons.h>

#include <sys/selinfo.h>
#include <machine/apm_bios.h>
#include <machine/pc/bios.h>
#include <machine/bus.h>
#include <i386/bios/apm.h>

extern int apm_display(int newstate);

extern struct apm_softc apm_softc;

static int blanked=0;

static int
apm_saver(video_adapter_t *adp, int blank)
{
	if (!apm_softc.initialized || !apm_softc.active)
		return 0;

	if (blank==blanked)
		return 0;

	blanked=blank;

	apm_display(!blanked);

	return 0;
}

static int
apm_init(video_adapter_t *adp)
{
	if (!apm_softc.initialized || !apm_softc.active)
		printf("WARNING: apm_saver module requires apm enabled\n");
	return 0;
}

static int
apm_term(video_adapter_t *adp)
{
	return 0;
}

static scrn_saver_t apm_module = {
	"apm_saver", apm_init, apm_term, apm_saver, NULL,
};

SAVER_MODULE(apm_saver, apm_module);
MODULE_DEPEND(apm_saver, apm, 1, 1, 1);