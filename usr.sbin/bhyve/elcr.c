
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

#include <sys/types.h>

#include "inout.h"

/*
 * EISA interrupt Level Control Register.
 *
 * This is a 16-bit register with one bit for each of the IRQ0 through IRQ15.
 * A level triggered irq is indicated by setting the corresponding bit to '1'.
 */
#define	ELCR_PORT	0x4d0

static uint8_t elcr[2] = { 0x00, 0x00 };

static int
elcr_handler(struct vmctx *ctx, int vcpu, int in, int port, int bytes,
	     uint32_t *eax, void *arg)
{
	int idx;

	if (bytes != 1)
		return (-1);

	idx = port - ELCR_PORT;

	if (in)
		*eax = elcr[idx];
	else
		elcr[idx] = *eax;

	return (0);
}
INOUT_PORT(elcr, ELCR_PORT + 0, IOPORT_F_INOUT, elcr_handler);
INOUT_PORT(elcr, ELCR_PORT + 1, IOPORT_F_INOUT, elcr_handler);