
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

#include <btxv86.h>

#include "rbx.h"
#include "util.h"
#include "drv.h"
#include "edd.h"
#ifdef USE_XREAD
#include "xreadorg.h"
#endif

#ifdef GPT
static struct edd_params params;

uint64_t
drvsize(struct dsk *dskp)
{

	params.len = sizeof(struct edd_params);
	v86.ctl = V86_FLAGS;
	v86.addr = 0x13;
	v86.eax = 0x4800;
	v86.edx = dskp->drive;
	v86.ds = VTOPSEG(&params);
	v86.esi = VTOPOFF(&params);
	v86int();
	if (V86_CY(v86.efl)) {
		printf("error %u\n", v86.eax >> 8 & 0xff);
		return (0);
	}
	return (params.sectors);
}
#endif	/* GPT */

#ifndef USE_XREAD
static struct edd_packet packet;
#endif

int
drvread(struct dsk *dskp, void *buf, daddr_t lba, unsigned nblk)
{
	static unsigned c = 0x2d5c7c2f;

	if (!OPT_CHECK(RBX_QUIET))
		printf("%c\b", c = c << 8 | c >> 24);
#ifndef USE_XREAD
	packet.len = sizeof(struct edd_packet);
	packet.count = nblk;
	packet.off = VTOPOFF(buf);
	packet.seg = VTOPSEG(buf);
	packet.lba = lba;
	v86.ctl = V86_FLAGS;
	v86.addr = 0x13;
	v86.eax = 0x4200;
	v86.edx = dskp->drive;
	v86.ds = VTOPSEG(&packet);
	v86.esi = VTOPOFF(&packet);
#else	/* USE_XREAD */
	v86.ctl = V86_ADDR | V86_CALLF | V86_FLAGS;
	v86.addr = XREADORG;		/* call to xread in boot1 */
	v86.es = VTOPSEG(buf);
	v86.eax = lba;
	v86.ebx = VTOPOFF(buf);
	v86.ecx = lba >> 32;
	v86.edx = nblk << 8 | dskp->drive;
#endif	/* USE_XREAD */
	v86int();
	if (V86_CY(v86.efl)) {
		printf("%s: error %u lba %u\n",
		    BOOTPROG, v86.eax >> 8 & 0xff, lba);
		return (-1);
	}
	return (0);
}

#ifdef GPT
int
drvwrite(struct dsk *dskp, void *buf, daddr_t lba, unsigned nblk)
{

	packet.len = sizeof(struct edd_packet);
	packet.count = nblk;
	packet.off = VTOPOFF(buf);
	packet.seg = VTOPSEG(buf);
	packet.lba = lba;
	v86.ctl = V86_FLAGS;
	v86.addr = 0x13;
	v86.eax = 0x4300;
	v86.edx = dskp->drive;
	v86.ds = VTOPSEG(&packet);
	v86.esi = VTOPOFF(&packet);
	v86int();
	if (V86_CY(v86.efl)) {
		printf("error %u lba %u\n", v86.eax >> 8 & 0xff, lba);
		return (-1);
	}
	return (0);
}
#endif	/* GPT */