
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
#include <sys/cdefs.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/conf.h>

#include <machine/xen/xen-os.h>

#include <xen/hypervisor.h>
#include <xen/xenstore/xenstorevar.h>
#include <xen/xenstore/xenstore_internal.h>

struct xs_dev_transaction {
	LIST_ENTRY(xs_dev_transaction) list;
	struct xs_transaction handle;
};

struct xs_dev_data {
	/* In-progress transaction. */
	LIST_HEAD(xdd_list_head, xs_dev_transaction) transactions;

	/* Partial request. */
	unsigned int len;
	union {
		struct xsd_sockmsg msg;
		char buffer[PAGE_SIZE];
	} u;

	/* Response queue. */
#define MASK_READ_IDX(idx) ((idx)&(PAGE_SIZE-1))
	char read_buffer[PAGE_SIZE];
	unsigned int read_cons, read_prod;
};

static int 
xs_dev_read(struct cdev *dev, struct uio *uio, int ioflag)
{
	int error;
	struct xs_dev_data *u = dev->si_drv1;

	while (u->read_prod == u->read_cons) {
		error = tsleep(u, PCATCH, "xsdread", hz/10);
		if (error && error != EWOULDBLOCK)
			return (error);
	}

	while (uio->uio_resid > 0) {
		if (u->read_cons == u->read_prod)
			break;
		error = uiomove(&u->read_buffer[MASK_READ_IDX(u->read_cons)],
		    1, uio);
		if (error)
			return (error);
		u->read_cons++;
	}
	return (0);
}

static void
xs_queue_reply(struct xs_dev_data *u, char *data, unsigned int len)
{
	int i;

	for (i = 0; i < len; i++, u->read_prod++)
		u->read_buffer[MASK_READ_IDX(u->read_prod)] = data[i];

	KASSERT((u->read_prod - u->read_cons) <= sizeof(u->read_buffer),
	    ("xenstore reply too big"));

	wakeup(u);
}

static int 
xs_dev_write(struct cdev *dev, struct uio *uio, int ioflag)
{
	int error;
	struct xs_dev_data *u = dev->si_drv1;
	struct xs_dev_transaction *trans;
	void *reply;
	int len = uio->uio_resid;

	if ((len + u->len) > sizeof(u->u.buffer))
		return (EINVAL);

	error = uiomove(u->u.buffer + u->len, len, uio);
	if (error)
		return (error);

	u->len += len;
	if (u->len < (sizeof(u->u.msg) + u->u.msg.len))
		return (0);

	switch (u->u.msg.type) {
	case XS_TRANSACTION_START:
	case XS_TRANSACTION_END:
	case XS_DIRECTORY:
	case XS_READ:
	case XS_GET_PERMS:
	case XS_RELEASE:
	case XS_GET_DOMAIN_PATH:
	case XS_WRITE:
	case XS_MKDIR:
	case XS_RM:
	case XS_SET_PERMS:
		error = xs_dev_request_and_reply(&u->u.msg, &reply);
		if (!error) {
			if (u->u.msg.type == XS_TRANSACTION_START) {
				trans = malloc(sizeof(*trans), M_XENSTORE,
				    M_WAITOK);
				trans->handle.id = strtoul(reply, NULL, 0);
				LIST_INSERT_HEAD(&u->transactions, trans, list);
			} else if (u->u.msg.type == XS_TRANSACTION_END) {
				LIST_FOREACH(trans, &u->transactions, list)
					if (trans->handle.id == u->u.msg.tx_id)
						break;
#if 0 /* XXX does this mean the list is empty? */
				BUG_ON(&trans->list == &u->transactions);
#endif
				LIST_REMOVE(trans, list);
				free(trans, M_XENSTORE);
			}
			xs_queue_reply(u, (char *)&u->u.msg, sizeof(u->u.msg));
			xs_queue_reply(u, (char *)reply, u->u.msg.len);
			free(reply, M_XENSTORE);
		}
		break;

	default:
		error = EINVAL;
		break;
	}

	if (error == 0)
		u->len = 0;

	return (error);
}

static int
xs_dev_open(struct cdev *dev, int oflags, int devtype, struct thread *td)
{
	struct xs_dev_data *u;

#if 0 /* XXX figure out if equiv needed */
	nonseekable_open(inode, filp);
#endif
	u = malloc(sizeof(*u), M_XENSTORE, M_WAITOK|M_ZERO);
	LIST_INIT(&u->transactions);
        dev->si_drv1 = u;

	return (0);
}

static int
xs_dev_close(struct cdev *dev, int fflag, int devtype, struct thread *td)
{
	struct xs_dev_data *u = dev->si_drv1;
	struct xs_dev_transaction *trans, *tmp;

	LIST_FOREACH_SAFE(trans, &u->transactions, list, tmp) {
		xs_transaction_end(trans->handle, 1);
		LIST_REMOVE(trans, list);
		free(trans, M_XENSTORE);
	}

	free(u, M_XENSTORE);
	return (0);
}

static struct cdevsw xs_dev_cdevsw = {
	.d_version = D_VERSION,	
	.d_read = xs_dev_read,
	.d_write = xs_dev_write,
	.d_open = xs_dev_open,
	.d_close = xs_dev_close,
	.d_name = "xs_dev",
};

void
xs_dev_init()
{
	make_dev(&xs_dev_cdevsw, 0, UID_ROOT, GID_WHEEL, 0400,
	    "xen/xenstore");
}