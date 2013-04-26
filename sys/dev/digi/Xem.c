
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

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/module.h>

#include <dev/digi/Xem.bios.h>
#include <dev/digi/Xem.fepos.h>
#include <dev/digi/digi_mod.h>

struct digi_mod digi_mod_Xem = {
	DIGI_MOD_VERSION,
	{ Xem_bios, sizeof(Xem_bios) },
	{ Xem_fepos, sizeof(Xem_fepos) },
	{ NULL, 0 }
};

MODULE_VERSION(digi_Xem, 1);
DEV_MODULE(digi_Xem, 0, 0);