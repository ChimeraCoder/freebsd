
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

#ifdef _KERNEL
#include <sys/malloc.h>
#else
#include <stdlib.h>
#include <string.h>
#endif

#include <rpc/rpc.h>
#include <rpc/rpc_com.h>

#include "gssd.h"

bool_t
xdr_gss_buffer_desc(XDR *xdrs, gss_buffer_desc *buf)
{
	char *val;
	u_int len;

	len = buf->length;
	val = buf->value;
	if (!xdr_bytes(xdrs, &val, &len, ~0))
		return (FALSE);
	buf->length = len;
	buf->value = val;

	return (TRUE);
}

bool_t
xdr_gss_OID_desc(XDR *xdrs, gss_OID_desc *oid)
{
	char *val;
	u_int len;

	len = oid->length;
	val = oid->elements;
	if (!xdr_bytes(xdrs, &val, &len, ~0))
		return (FALSE);
	oid->length = len;
	oid->elements = val;

	return (TRUE);
}

bool_t
xdr_gss_OID(XDR *xdrs, gss_OID *oidp)
{
	gss_OID oid;
	bool_t is_null;

	switch (xdrs->x_op) {
	case XDR_ENCODE:
		oid = *oidp;
		if (oid) {
			is_null = FALSE;
			if (!xdr_bool(xdrs, &is_null)
			    || !xdr_gss_OID_desc(xdrs, oid))
				return (FALSE);
		} else {
			is_null = TRUE;
			if (!xdr_bool(xdrs, &is_null))
				return (FALSE);
		}
		break;

	case XDR_DECODE:
		if (!xdr_bool(xdrs, &is_null))
			return (FALSE);
		if (is_null) {
			*oidp = GSS_C_NO_OID;
		} else {
			oid = mem_alloc(sizeof(gss_OID_desc));
			memset(oid, 0, sizeof(*oid));
			if (!xdr_gss_OID_desc(xdrs, oid))
				return (FALSE);
			*oidp = oid;
		}
		break;

	case XDR_FREE:
		oid = *oidp;
		if (oid) {
			xdr_gss_OID_desc(xdrs, oid);
			mem_free(oid, sizeof(gss_OID_desc));
		}
	}

	return (TRUE);
}

bool_t
xdr_gss_OID_set_desc(XDR *xdrs, gss_OID_set_desc *set)
{
	caddr_t addr;
	u_int len;

	len = set->count;
	addr = (caddr_t) set->elements;
	if (!xdr_array(xdrs, &addr, &len, ~0, sizeof(gss_OID_desc),
		(xdrproc_t) xdr_gss_OID_desc))
		return (FALSE);
	set->count = len;
	set->elements = (gss_OID) addr;

	return (TRUE);
}

bool_t
xdr_gss_OID_set(XDR *xdrs, gss_OID_set *setp)
{
	gss_OID_set set;
	bool_t is_null;

	switch (xdrs->x_op) {
	case XDR_ENCODE:
		set = *setp;
		if (set) {
			is_null = FALSE;
			if (!xdr_bool(xdrs, &is_null)
			    || !xdr_gss_OID_set_desc(xdrs, set))
				return (FALSE);
		} else {
			is_null = TRUE;
			if (!xdr_bool(xdrs, &is_null))
				return (FALSE);
		}
		break;

	case XDR_DECODE:
		if (!xdr_bool(xdrs, &is_null))
			return (FALSE);
		if (is_null) {
			*setp = GSS_C_NO_OID_SET;
		} else {
			set = mem_alloc(sizeof(gss_OID_set_desc));
			memset(set, 0, sizeof(*set));
			if (!xdr_gss_OID_set_desc(xdrs, set))
				return (FALSE);
			*setp = set;
		}
		break;

	case XDR_FREE:
		set = *setp;
		if (set) {
			xdr_gss_OID_set_desc(xdrs, set);
			mem_free(set, sizeof(gss_OID_set_desc));
		}
	}

	return (TRUE);
}

bool_t
xdr_gss_channel_bindings_t(XDR *xdrs, gss_channel_bindings_t *chp)
{
	gss_channel_bindings_t ch;
	bool_t is_null;

	switch (xdrs->x_op) {
	case XDR_ENCODE:
		ch = *chp;
		if (ch) {
			is_null = FALSE;
			if (!xdr_bool(xdrs, &is_null)
			    || !xdr_uint32_t(xdrs, &ch->initiator_addrtype)
			    || !xdr_gss_buffer_desc(xdrs,
				&ch->initiator_address)
			    || !xdr_uint32_t(xdrs, &ch->acceptor_addrtype)
			    || !xdr_gss_buffer_desc(xdrs,
				&ch->acceptor_address)
			    || !xdr_gss_buffer_desc(xdrs,
				&ch->application_data))
				return (FALSE);
		} else {
			is_null = TRUE;
			if (!xdr_bool(xdrs, &is_null))
				return (FALSE);
		}
		break;

	case XDR_DECODE:
		if (!xdr_bool(xdrs, &is_null))
			return (FALSE);
		if (is_null) {
			*chp = GSS_C_NO_CHANNEL_BINDINGS;
		} else {
			ch = mem_alloc(sizeof(*ch));
			memset(ch, 0, sizeof(*ch));
			if (!xdr_uint32_t(xdrs, &ch->initiator_addrtype)
			    || !xdr_gss_buffer_desc(xdrs,
				&ch->initiator_address)
			    || !xdr_uint32_t(xdrs, &ch->acceptor_addrtype)
			    || !xdr_gss_buffer_desc(xdrs,
				&ch->acceptor_address)
			    || !xdr_gss_buffer_desc(xdrs,
				&ch->application_data))
				return (FALSE);
			*chp = ch;
		}
		break;

	case XDR_FREE:
		ch = *chp;
		if (ch) {
			xdr_gss_buffer_desc(xdrs, &ch->initiator_address);
			xdr_gss_buffer_desc(xdrs, &ch->acceptor_address);
			xdr_gss_buffer_desc(xdrs, &ch->application_data);
			mem_free(ch, sizeof(*ch));
		}
	}

	return (TRUE);
}