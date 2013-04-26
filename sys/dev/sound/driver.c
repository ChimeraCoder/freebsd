
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

#ifdef HAVE_KERNEL_OPTION_HEADERS
#include "opt_snd.h"
#endif

#include <dev/sound/pcm/sound.h>

static int
snd_modevent(module_t mod, int type, void *data)
{

	switch (type) {
	case MOD_LOAD:
		break;
	case MOD_UNLOAD:
		break;
	default:
		return (ENOTSUP);
		break;
	}
	return 0;
}

static moduledata_t snd_mod = {
	"snd_driver",
	snd_modevent,
	NULL
};
DECLARE_MODULE(snd_driver, snd_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
MODULE_VERSION(snd_driver, 1);

MODULE_DEPEND(snd_driver, snd_ad1816, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_als4000, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_atiixp, 1, 1, 1);
/* MODULE_DEPEND(snd_driver, snd_aureal, 1, 1, 1); */
MODULE_DEPEND(snd_driver, snd_cmi, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_cs4281, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_csa, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_csapcm, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_ds1, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_emu10kx, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_envy24, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_envy24ht, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_es137x, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_ess, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_fm801, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_gusc, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_hda, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_ich, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_maestro, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_maestro3, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_mss, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_neomagic, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_sb16, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_sb8, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_sbc, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_solo, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_spicds, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_t4dwave, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_via8233, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_via82c686, 1, 1, 1);
MODULE_DEPEND(snd_driver, snd_vibes, 1, 1, 1);