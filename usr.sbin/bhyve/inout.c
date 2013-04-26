
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
#include <sys/linker_set.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "inout.h"

SET_DECLARE(inout_port_set, struct inout_port);

#define	MAX_IOPORTS	(1 << 16)

#define	VERIFY_IOPORT(port, size) \
	assert((port) >= 0 && (size) > 0 && ((port) + (size)) <= MAX_IOPORTS)

static struct {
	const char	*name;
	int		flags;
	inout_func_t	handler;
	void		*arg;
} inout_handlers[MAX_IOPORTS];

static int
default_inout(struct vmctx *ctx, int vcpu, int in, int port, int bytes,
              uint32_t *eax, void *arg)
{
        if (in) {
                switch (bytes) {
                case 4:
                        *eax = 0xffffffff;
                        break;
                case 2:
                        *eax = 0xffff;
                        break;
                case 1:
                        *eax = 0xff;
                        break;
                }
        }
        
        return (0);
}

static void 
register_default_iohandler(int start, int size)
{
	struct inout_port iop;
	
	VERIFY_IOPORT(start, size);

	bzero(&iop, sizeof(iop));
	iop.name = "default";
	iop.port = start;
	iop.size = size;
	iop.flags = IOPORT_F_INOUT;
	iop.handler = default_inout;

	register_inout(&iop);
}

int
emulate_inout(struct vmctx *ctx, int vcpu, int in, int port, int bytes,
	      uint32_t *eax, int strict)
{
	int flags;
	uint32_t mask;
	inout_func_t handler;
	void *arg;

	assert(port < MAX_IOPORTS);

	handler = inout_handlers[port].handler;

	if (strict && handler == default_inout)
		return (-1);

	if (!in) {
		switch (bytes) {
		case 1:
			mask = 0xff;
			break;
		case 2:
			mask = 0xffff;
			break;
		default:
			mask = 0xffffffff;
			break;
		}
		*eax = *eax & mask;
	}

	flags = inout_handlers[port].flags;
	arg = inout_handlers[port].arg;

	if ((in && (flags & IOPORT_F_IN)) || (!in && (flags & IOPORT_F_OUT)))
		return ((*handler)(ctx, vcpu, in, port, bytes, eax, arg));
	else
		return (-1);
}

void
init_inout(void)
{
	struct inout_port **iopp, *iop;

	/*
	 * Set up the default handler for all ports
	 */
	register_default_iohandler(0, MAX_IOPORTS);

	/*
	 * Overwrite with specified handlers
	 */
	SET_FOREACH(iopp, inout_port_set) {
		iop = *iopp;
		assert(iop->port < MAX_IOPORTS);
		inout_handlers[iop->port].name = iop->name;
		inout_handlers[iop->port].flags = iop->flags;
		inout_handlers[iop->port].handler = iop->handler;
		inout_handlers[iop->port].arg = NULL;
	}
}

int
register_inout(struct inout_port *iop)
{
	int i;

	VERIFY_IOPORT(iop->port, iop->size);
	
	for (i = iop->port; i < iop->port + iop->size; i++) {
		inout_handlers[i].name = iop->name;
		inout_handlers[i].flags = iop->flags;
		inout_handlers[i].handler = iop->handler;
		inout_handlers[i].arg = iop->arg;
	}

	return (0);
}

int
unregister_inout(struct inout_port *iop)
{

	VERIFY_IOPORT(iop->port, iop->size);
	assert(inout_handlers[iop->port].name == iop->name);

	register_default_iohandler(iop->port, iop->size);

	return (0);
}