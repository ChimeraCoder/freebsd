
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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "inout.h"

/*
 * FreeBSD only writes to the 8259 interrupt controllers to put them in a
 * shutdown state.
 *
 * So, we just ignore the writes.
 */

#define	IO_ICU1		0x20
#define	IO_ICU2		0xA0
#define	ICU_IMR_OFFSET	1

static int
atpic_handler(struct vmctx *ctx, int vcpu, int in, int port, int bytes,
	      uint32_t *eax, void *arg)
{
	if (bytes != 1)
		return (-1);

	if (in)
		return (-1);

	/* Pretend all writes to the 8259 are alright */
	return (0);
}

INOUT_PORT(atpic, IO_ICU1, IOPORT_F_INOUT, atpic_handler);
INOUT_PORT(atpic, IO_ICU1 + ICU_IMR_OFFSET, IOPORT_F_INOUT, atpic_handler);
INOUT_PORT(atpic, IO_ICU2, IOPORT_F_INOUT, atpic_handler);
INOUT_PORT(atpic, IO_ICU2 + ICU_IMR_OFFSET, IOPORT_F_INOUT, atpic_handler);