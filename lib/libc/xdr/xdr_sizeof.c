
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

#include "namespace.h"
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <sys/types.h>
#include <stdlib.h>
#include "un-namespace.h"

/* ARGSUSED */
static bool_t
x_putlong(xdrs, longp)
	XDR *xdrs;
	long *longp;
{
	xdrs->x_handy += BYTES_PER_XDR_UNIT;
	return (TRUE);
}

/* ARGSUSED */
static bool_t
x_putbytes(xdrs, bp, len)
	XDR *xdrs;
	char  *bp;
	u_int len;
{
	xdrs->x_handy += len;
	return (TRUE);
}

static u_int
x_getpostn(xdrs)
	XDR *xdrs;
{
	return (xdrs->x_handy);
}

/* ARGSUSED */
static bool_t
x_setpostn(xdrs, pos)
	XDR *xdrs;
	u_int pos;
{
	/* This is not allowed */
	return (FALSE);
}

static int32_t *
x_inline(xdrs, len)
	XDR *xdrs;
	u_int len;
{
	if (len == 0) {
		return (NULL);
	}
	if (xdrs->x_op != XDR_ENCODE) {
		return (NULL);
	}
	if (len < (u_int)(uintptr_t)xdrs->x_base) {
		/* x_private was already allocated */
		xdrs->x_handy += len;
		return ((int32_t *) xdrs->x_private);
	} else {
		/* Free the earlier space and allocate new area */
		if (xdrs->x_private)
			free(xdrs->x_private);
		if ((xdrs->x_private = (caddr_t) malloc(len)) == NULL) {
			xdrs->x_base = 0;
			return (NULL);
		}
		xdrs->x_base = (caddr_t)(uintptr_t)len;
		xdrs->x_handy += len;
		return ((int32_t *) xdrs->x_private);
	}
}

static int
harmless()
{
	/* Always return FALSE/NULL, as the case may be */
	return (0);
}

static void
x_destroy(xdrs)
	XDR *xdrs;
{
	xdrs->x_handy = 0;
	xdrs->x_base = 0;
	if (xdrs->x_private) {
		free(xdrs->x_private);
		xdrs->x_private = NULL;
	}
	return;
}

unsigned long
xdr_sizeof(func, data)
	xdrproc_t func;
	void *data;
{
	XDR x;
	struct xdr_ops ops;
	bool_t stat;
	/* to stop ANSI-C compiler from complaining */
	typedef  bool_t (* dummyfunc1)(XDR *, long *);
	typedef  bool_t (* dummyfunc2)(XDR *, caddr_t, u_int);

	ops.x_putlong = x_putlong;
	ops.x_putbytes = x_putbytes;
	ops.x_inline = x_inline;
	ops.x_getpostn = x_getpostn;
	ops.x_setpostn = x_setpostn;
	ops.x_destroy = x_destroy;

	/* the other harmless ones */
	ops.x_getlong =  (dummyfunc1) harmless;
	ops.x_getbytes = (dummyfunc2) harmless;

	x.x_op = XDR_ENCODE;
	x.x_ops = &ops;
	x.x_handy = 0;
	x.x_private = (caddr_t) NULL;
	x.x_base = (caddr_t) 0;

	stat = func(&x, data);
	if (x.x_private)
		free(x.x_private);
	return (stat == TRUE ? (unsigned) x.x_handy: 0);
}