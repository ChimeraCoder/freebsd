
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

#include "qla_os.h"
#include "qla_reg.h"
#include "qla_hw.h"
#include "qla_def.h"
#include "qla_inline.h"
#include "qla_ver.h"
#include "qla_glbl.h"
#include "qla_dbg.h"


uint32_t dbg_level = 0 ;
/*
 * Name: qla_dump_buf32
 * Function: dumps a buffer as 32 bit words
 */
void qla_dump_buf32(qla_host_t *ha, char *msg, void *dbuf32, uint32_t len32)
{
        device_t dev;
	uint32_t i = 0;
	uint32_t *buf;

        dev = ha->pci_dev;
	buf = dbuf32;

	device_printf(dev, "%s: %s dump start\n", __func__, msg);

	while (len32 >= 4) {
		device_printf(dev,"0x%08x: 0x%08x 0x%08x 0x%08x 0x%08x\n",
			i, buf[0], buf[1], buf[2], buf[3]);
		i += 4 * 4;
		len32 -= 4;
		buf += 4;
	}
	switch (len32) {
	case 1:
		device_printf(dev,"0x%08x: 0x%08x\n", i, buf[0]);
		break;
	case 2:
		device_printf(dev,"0x%08x: 0x%08x 0x%08x\n", i, buf[0], buf[1]);
		break;
	case 3:
		device_printf(dev,"0x%08x: 0x%08x 0x%08x 0x%08x\n",
			i, buf[0], buf[1], buf[2]);
		break;
	default:
		break;
	}
	device_printf(dev, "%s: %s dump end\n", __func__, msg);
}

/*
 * Name: qla_dump_buf16
 * Function: dumps a buffer as 16 bit words
 */
void qla_dump_buf16(qla_host_t *ha, char *msg, void *dbuf16, uint32_t len16)
{
        device_t dev;
	uint32_t i = 0;
	uint16_t *buf;

        dev = ha->pci_dev;
	buf = dbuf16;

	device_printf(dev, "%s: %s dump start\n", __func__, msg);

	while (len16 >= 8) {
		device_printf(dev,"0x%08x: 0x%04x 0x%04x 0x%04x 0x%04x"
			" 0x%04x 0x%04x 0x%04x 0x%04x\n", i, buf[0],
			buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
		i += 16;
		len16 -= 8;
		buf += 8;
	}
	switch (len16) {
	case 1:
		device_printf(dev,"0x%08x: 0x%04x\n", i, buf[0]);
		break;
	case 2:
		device_printf(dev,"0x%08x: 0x%04x 0x%04x\n", i, buf[0], buf[1]);
		break;
	case 3:
		device_printf(dev,"0x%08x: 0x%04x 0x%04x 0x%04x\n",
			i, buf[0], buf[1], buf[2]);
		break;
	case 4:
		device_printf(dev,"0x%08x: 0x%04x 0x%04x 0x%04x 0x%04x\n", i,
			buf[0], buf[1], buf[2], buf[3]);
		break;
	case 5:
		device_printf(dev,"0x%08x:"
			" 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x\n", i,
			buf[0], buf[1], buf[2], buf[3], buf[4]);
		break;
	case 6:
		device_printf(dev,"0x%08x:"
			" 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x\n", i,
			buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
		break;
	case 7:
		device_printf(dev,"0x%04x: 0x%04x 0x%04x 0x%04x 0x%04x"
			" 0x%04x 0x%04x 0x%04x\n", i, buf[0], buf[1],
			buf[2], buf[3], buf[4], buf[5], buf[6]);
		break;
	default:
		break;
	}
	device_printf(dev, "%s: %s dump end\n", __func__, msg);
}

/*
 * Name: qla_dump_buf8
 * Function: dumps a buffer as bytes
 */
void qla_dump_buf8(qla_host_t *ha, char *msg, void *dbuf, uint32_t len)
{
        device_t dev;
	uint32_t i = 0;
	uint8_t *buf;

        dev = ha->pci_dev;
	buf = dbuf;

	device_printf(dev, "%s: %s 0x%x dump start\n", __func__, msg, len);
	
	while (len >= 16) {
		device_printf(dev,"0x%08x:"
			" %02x %02x %02x %02x %02x %02x %02x %02x"
			" %02x %02x %02x %02x %02x %02x %02x %02x\n", i,
			buf[0], buf[1], buf[2], buf[3],
			buf[4], buf[5], buf[6], buf[7],
			buf[8], buf[9], buf[10], buf[11],
			buf[12], buf[13], buf[14], buf[15]);
		i += 16;
		len -= 16;
		buf += 16;
	}
	switch (len) {
	case 1:
		device_printf(dev,"0x%08x: %02x\n", i, buf[0]);
		break;
	case 2:
		device_printf(dev,"0x%08x: %02x %02x\n", i, buf[0], buf[1]);
		break;
	case 3:
		device_printf(dev,"0x%08x: %02x %02x %02x\n",
			i, buf[0], buf[1], buf[2]);
		break;
	case 4:
		device_printf(dev,"0x%08x: %02x %02x %02x %02x\n", i,
			buf[0], buf[1], buf[2], buf[3]);
		break;
	case 5:
		device_printf(dev,"0x%08x:"
			" %02x %02x %02x %02x %02x\n", i,
			buf[0], buf[1], buf[2], buf[3], buf[4]);
		break;
	case 6:
		device_printf(dev,"0x%08x:"
			" %02x %02x %02x %02x %02x %02x\n", i,
			buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
		break;
	case 7:
		device_printf(dev,"0x%08x:"
			" %02x %02x %02x %02x %02x %02x %02x\n", i,
			buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]);
		break;
	case 8:
		device_printf(dev,"0x%08x:"
			" %02x %02x %02x %02x %02x %02x %02x %02x\n", i,
			buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6],
			buf[7]);
		break;
	case 9:
		device_printf(dev,"0x%08x:"
			" %02x %02x %02x %02x %02x %02x %02x %02x"
			" %02x\n", i,
			buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6],
			buf[7], buf[8]);
		break;
	case 10:
		device_printf(dev,"0x%08x:"
			" %02x %02x %02x %02x %02x %02x %02x %02x"
			" %02x %02x\n", i,
			buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6],
			buf[7], buf[8], buf[9]);
		break;
	case 11:
		device_printf(dev,"0x%08x:"
			" %02x %02x %02x %02x %02x %02x %02x %02x"
			" %02x %02x %02x\n", i,
			buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6],
			buf[7], buf[8], buf[9], buf[10]);
		break;
	case 12:
		device_printf(dev,"0x%08x:"
			" %02x %02x %02x %02x %02x %02x %02x %02x"
			" %02x %02x %02x %02x\n", i,
			buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6],
			buf[7], buf[8], buf[9], buf[10], buf[11]);
		break;
	case 13:
		device_printf(dev,"0x%08x:"
			" %02x %02x %02x %02x %02x %02x %02x %02x"
			" %02x %02x %02x %02x %02x\n", i,
			buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6],
			buf[7], buf[8], buf[9], buf[10], buf[11], buf[12]);
		break;
	case 14:
		device_printf(dev,"0x%08x:"
			" %02x %02x %02x %02x %02x %02x %02x %02x"
			" %02x %02x %02x %02x %02x %02x\n", i,
			buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6],
			buf[7], buf[8], buf[9], buf[10], buf[11], buf[12],
			buf[13]);
		break;
	case 15:
		device_printf(dev,"0x%08x:"
			" %02x %02x %02x %02x %02x %02x %02x %02x"
			" %02x %02x %02x %02x %02x %02x %02x\n", i,
			buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6],
			buf[7], buf[8], buf[9], buf[10], buf[11], buf[12],
			buf[13], buf[14]);
		break;
	default:
		break;
	}
	
	device_printf(dev, "%s: %s dump end\n", __func__, msg);
}