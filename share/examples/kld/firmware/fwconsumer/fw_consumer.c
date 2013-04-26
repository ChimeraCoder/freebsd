
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
#include <sys/kernel.h>
#include <sys/errno.h>
#include <sys/systm.h>
#include <sys/linker.h>
#include <sys/firmware.h>
#include <sys/proc.h>
#include <sys/module.h>

static const struct firmware *fp;

static int
fw_consumer_modevent(module_t mod, int type, void *unused)
{
	switch (type) {
	case MOD_LOAD:
		fp = firmware_get("beastie");

		if (fp == NULL)
			return (ENOENT);

		if (((const char *)fp->data)[fp->datasize - 1] != '\0') {
			firmware_put(fp, FIRMWARE_UNLOAD);
			return (EINVAL);
		}
		printf("%s", (const char *)fp->data);

		return (0);
	case MOD_UNLOAD:
		printf("Bye!\n");

		if (fp != NULL) {
			printf("%s", (const char *)fp->data);
			firmware_put(fp, FIRMWARE_UNLOAD);
		}

		return (0);
	}
	return (EINVAL);
}

static moduledata_t fw_consumer_mod = {
	"fw_consumer",
	fw_consumer_modevent,
	0
};
DECLARE_MODULE(fw_consumer, fw_consumer_mod, SI_SUB_DRIVERS, SI_ORDER_ANY);
MODULE_VERSION(fw_consumer, 1);
MODULE_DEPEND(fw_consumer, firmware, 1, 1, 1);