
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
#include <sys/bus.h>

#include <isa/isareg.h>
#include <isa/isavar.h>

/*
 * Glue code to load sound(4). Though fdc(4), ppc(4) don't work on
 * sparc64 yet, they may need this glue code too.
 */

int
isa_dma_init(int chan, u_int bouncebufsize, int flag)
{

	return (0);
}

int
isa_dma_acquire(int chan)
{
	return (0);
}

void
isa_dma_release(int chan)
{

}

void
isa_dmacascade(int chan)
{

}

void
isa_dmastart(int flags, caddr_t addr, u_int nbytes, int chan)
{

}

void
isa_dmadone(int flags, caddr_t addr, int nbytes, int chan)
{

}

int
isa_dmastatus(int chan)
{
	return (0);
}

int
isa_dmatc(int chan)
{
	return (0);
}

int
isa_dmastop(int chan)
{
	return (0);
}