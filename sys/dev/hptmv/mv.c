
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

#include <sys/time.h>
#include <sys/kernel.h>
#include <sys/bus.h>
#include <machine/resource.h>

#if __FreeBSD_version >= 500043
#include <machine/pci_cfgreg.h>
#endif

#if (__FreeBSD_version < 500043)
#include <sys/bus_private.h>
#endif

#if (__FreeBSD_version < 500000)
#include <machine/clock.h>
#endif

#ifndef __KERNEL__
#define __KERNEL__
#endif

#include <dev/hptmv/global.h>
#include <dev/hptmv/hptintf.h>
#include <dev/hptmv/mvOs.h>
#include <dev/hptmv/osbsd.h>


void HPTLIBAPI
MV_REG_WRITE_BYTE(MV_BUS_ADDR_T base, MV_U32 offset, MV_U8 val)
{ 
	writeb((void *)((ULONG_PTR)base + offset), val); 
}

void HPTLIBAPI
MV_REG_WRITE_WORD(MV_BUS_ADDR_T base, MV_U32 offset, MV_U16 val)
{ 
	writew((void *)((ULONG_PTR)base + offset), val); 
}

void HPTLIBAPI
MV_REG_WRITE_DWORD(MV_BUS_ADDR_T base, MV_U32 offset, MV_U32 val)
{
	writel((void *)((ULONG_PTR)base + offset), val);
}

MV_U8  HPTLIBAPI
MV_REG_READ_BYTE(MV_BUS_ADDR_T base, MV_U32 offset)
{
	return readb((void *)((ULONG_PTR)base + offset));
}

MV_U16 HPTLIBAPI
MV_REG_READ_WORD(MV_BUS_ADDR_T base, MV_U32 offset)
{
	return readw((void *)((ULONG_PTR)base + offset));
}

MV_U32 HPTLIBAPI
MV_REG_READ_DWORD(MV_BUS_ADDR_T base, MV_U32 offset)
{
	return readl((void *)((ULONG_PTR)base + offset));
}

int HPTLIBAPI
os_memcmp(const void *cs, const void *ct, unsigned len)
{
	return memcmp(cs, ct, len);
}

void HPTLIBAPI
os_memcpy(void *to, const void *from, unsigned len)
{
	memcpy(to, from, len);
}

void HPTLIBAPI
os_memset(void *s, char c, unsigned len)
{
	memset(s, c, len);
}

unsigned HPTLIBAPI
os_strlen(const char *s)
{
	return strlen(s);
}

void HPTLIBAPI
mvMicroSecondsDelay(MV_U32 msecs)
{
	DELAY(msecs);
}

ULONG_PTR HPTLIBAPI
fOsPhysicalAddress(void *addr)
{
	return (ULONG_PTR)(vtophys(addr));
}