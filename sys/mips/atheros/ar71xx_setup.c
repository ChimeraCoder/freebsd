
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

#include "opt_ddb.h"

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/cons.h>
#include <sys/kdb.h>
#include <sys/reboot.h>
 
#include <vm/vm.h>
#include <vm/vm_page.h>
 
#include <net/ethernet.h>
 
#include <machine/clock.h>
#include <machine/cpu.h>
#include <machine/cpuregs.h>
#include <machine/hwfunc.h>
#include <machine/md_var.h>
#include <machine/trap.h>
#include <machine/vmparam.h>
 
#include <mips/atheros/ar71xxreg.h>
#include <mips/atheros/ar933xreg.h>

#include <mips/atheros/ar71xx_setup.h>

#include <mips/atheros/ar71xx_cpudef.h>

#include <mips/atheros/ar71xx_chip.h>
#include <mips/atheros/ar724x_chip.h>
#include <mips/atheros/ar91xx_chip.h>
#include <mips/atheros/ar933x_chip.h>

#define	AR71XX_SYS_TYPE_LEN		128

static char ar71xx_sys_type[AR71XX_SYS_TYPE_LEN];
enum ar71xx_soc_type ar71xx_soc;
struct ar71xx_cpu_def * ar71xx_cpu_ops = NULL;

void
ar71xx_detect_sys_type(void)
{
	char *chip = "????";
	uint32_t id;
	uint32_t major;
	uint32_t minor;
	uint32_t rev = 0;

	id = ATH_READ_REG(AR71XX_RST_RESET_REG_REV_ID);
	major = id & REV_ID_MAJOR_MASK;

	switch (major) {
	case REV_ID_MAJOR_AR71XX:
		minor = id & AR71XX_REV_ID_MINOR_MASK;
		rev = id >> AR71XX_REV_ID_REVISION_SHIFT;
		rev &= AR71XX_REV_ID_REVISION_MASK;
		ar71xx_cpu_ops = &ar71xx_chip_def;
		switch (minor) {
		case AR71XX_REV_ID_MINOR_AR7130:
			ar71xx_soc = AR71XX_SOC_AR7130;
			chip = "7130";
			break;

		case AR71XX_REV_ID_MINOR_AR7141:
			ar71xx_soc = AR71XX_SOC_AR7141;
			chip = "7141";
			break;

		case AR71XX_REV_ID_MINOR_AR7161:
			ar71xx_soc = AR71XX_SOC_AR7161;
			chip = "7161";
			break;
		}
		break;

	case REV_ID_MAJOR_AR7240:
		ar71xx_soc = AR71XX_SOC_AR7240;
		chip = "7240";
		ar71xx_cpu_ops = &ar724x_chip_def;
		rev = (id & AR724X_REV_ID_REVISION_MASK);
		break;

	case REV_ID_MAJOR_AR7241:
		ar71xx_soc = AR71XX_SOC_AR7241;
		chip = "7241";
		ar71xx_cpu_ops = &ar724x_chip_def;
		rev = (id & AR724X_REV_ID_REVISION_MASK);
		break;

	case REV_ID_MAJOR_AR7242:
		ar71xx_soc = AR71XX_SOC_AR7242;
		chip = "7242";
		ar71xx_cpu_ops = &ar724x_chip_def;
		rev = (id & AR724X_REV_ID_REVISION_MASK);
		break;

	case REV_ID_MAJOR_AR913X:
		minor = id & AR91XX_REV_ID_MINOR_MASK;
		rev = id >> AR91XX_REV_ID_REVISION_SHIFT;
		rev &= AR91XX_REV_ID_REVISION_MASK;
		ar71xx_cpu_ops = &ar91xx_chip_def;
		switch (minor) {
		case AR91XX_REV_ID_MINOR_AR9130:
			ar71xx_soc = AR71XX_SOC_AR9130;
			chip = "9130";
			break;

		case AR91XX_REV_ID_MINOR_AR9132:
			ar71xx_soc = AR71XX_SOC_AR9132;
			chip = "9132";
			break;
		}
		break;
	case REV_ID_MAJOR_AR9330:
		minor = 0;
		rev = (id & AR933X_REV_ID_REVISION_MASK);
		chip = "9330";
		ar71xx_cpu_ops = &ar933x_chip_def;
		ar71xx_soc = AR71XX_SOC_AR9330;
		break;
	case REV_ID_MAJOR_AR9331:
		minor = 1;
		rev = (id & AR933X_REV_ID_REVISION_MASK);
		chip = "9331";
		ar71xx_soc = AR71XX_SOC_AR9331;
		ar71xx_cpu_ops = &ar933x_chip_def;
		break;

	default:
		panic("ar71xx: unknown chip id:0x%08x\n", id);
	}

	sprintf(ar71xx_sys_type, "Atheros AR%s rev %u", chip, rev);
}

const char *
ar71xx_get_system_type(void)
{
	return ar71xx_sys_type;
}