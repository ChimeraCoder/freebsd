
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

#include <assert.h>

#include "inout.h"

static int
post_data_handler(struct vmctx *ctx, int vcpu, int in, int port, int bytes,
		  uint32_t *eax, void *arg)
{
	assert(in == 1);

	if (bytes != 1)
		return (-1);

	*eax = 0xff;		/* return some garbage */
	return (0);
}

INOUT_PORT(post, 0x84, IOPORT_F_IN, post_data_handler);