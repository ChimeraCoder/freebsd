
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
#include <sys/systm.h>
#include <sys/endian.h>

#include <mips/nlm/hal/mips-extns.h>
#include <mips/nlm/hal/haldefs.h>
#include <mips/nlm/hal/iomap.h>
#include <mips/nlm/hal/gbu.h>

#include <mips/nlm/board.h>

#define CPLD_REVISION		0x0
#define CPLD_RESET		0x1
#define CPLD_CTRL		0x2
#define CPLD_RSVD		0x3
#define CPLD_PWR_CTRL		0x4
#define CPLD_MISC		0x5
#define CPLD_CTRL_STATUS	0x6
#define CPLD_PWR_INTR_STATUS	0x7
#define CPLD_DATA		0x8

static __inline
int nlm_cpld_read(uint64_t base, int reg)
{
	uint16_t val;

	val = *(volatile uint16_t *)(long)(base + reg * 2);
	return bswap16(val);
}

static __inline void
nlm_cpld_write(uint64_t base, int reg, uint16_t data)
{
	bswap16(data);
	*(volatile uint16_t *)(long)(base + reg * 2) = data;
}

int
nlm_board_cpld_majorversion(uint64_t base)
{
	return (nlm_cpld_read(base, CPLD_REVISION) >> 8);
}

int
nlm_board_cpld_minorversion(uint64_t base)
{
	return (nlm_cpld_read(base, CPLD_REVISION) & 0xff);
}

uint64_t nlm_board_cpld_base(int node, int chipselect)
{
	uint64_t gbubase, cpld_phys;

	gbubase = nlm_get_gbu_regbase(node);
	cpld_phys = nlm_read_gbu_reg(gbubase, GBU_CS_BASEADDR(chipselect));
	return (MIPS_PHYS_TO_KSEG1(cpld_phys << 8));
}

void
nlm_board_cpld_reset(uint64_t base)
{

	nlm_cpld_write(base, CPLD_RESET, 1 << 15);
	for(;;)
		__asm __volatile("wait");
}

/* get daughter board type */
int
nlm_board_cpld_dboard_type(uint64_t base, int slot)
{
	uint16_t val;
	int shift = 0;

	switch (slot) {
	case 0: shift = 0; break;
	case 1: shift = 4; break;
	case 2: shift = 2; break;
	case 3: shift = 6; break;
	}
	val = nlm_cpld_read(base, CPLD_CTRL_STATUS) >> shift;
	return (val & 0x3);
}