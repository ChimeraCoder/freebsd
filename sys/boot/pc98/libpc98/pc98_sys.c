
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

#include <btxv86.h>
#include <machine/cpufunc.h>
#define _KERNEL
#include <pc98/pc98/pc98_machdep.h>

/*
 * Set machine type to PC98_SYSTEM_PARAMETER.
 */
void
set_machine_type(void)
{
	int i;
	u_long ret, data;

	/* PC98_SYSTEM_PARAMETER (0x501) */
	ret = ((*(u_char *)PTOV(0xA1501)) & 0x08) >> 3;

	/* Wait V-SYNC */
	while (inb(0x60) & 0x20) {}
	while (!(inb(0x60) & 0x20)) {}

	/* ANK 'A' font */
	outb(0xa1, 0x00);
	outb(0xa3, 0x41);

	/* M_NORMAL, use CG window (all NEC OK)  */
	for (i = data = 0; i < 4; i++)
		data += *((u_long *)PTOV(0xA4000) + i);	/* 0xa4000 */
	if (data == 0x6efc58fc)		/* DA data */
		ret |= M_NEC_PC98;
	else
		ret |= M_EPSON_PC98;
	ret |= (inb(0x42) & 0x20) ? M_8M : 0;

	/* PC98_SYSTEM_PARAMETER(0x400) */
	if ((*(u_char *)PTOV(0xA1400)) & 0x80)
		ret |= M_NOTE;
	if (ret & M_NEC_PC98) {
		/* PC98_SYSTEM_PARAMETER(0x458) */
		if ((*(u_char *)PTOV(0xA1458)) & 0x80)
			ret |= M_H98;
		else
			ret |= M_NOT_H98;
	} else
		ret |= M_NOT_H98;

	(*(u_long *)PTOV(0xA1620)) = ret;
}