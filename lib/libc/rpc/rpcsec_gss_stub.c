
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

#include <rpc/rpc.h>
#include <rpc/rpcsec_gss.h>

bool_t
__rpc_gss_wrap_stub(AUTH *auth, void *header, size_t headerlen, XDR* xdrs,
    xdrproc_t xdr_args, void *args_ptr)
{

	return (FALSE);
}

bool_t
__rpc_gss_unwrap_stub(AUTH *auth, XDR* xdrs, xdrproc_t xdr_args, void *args_ptr)
{

	return (FALSE);
}

__weak_reference(__rpc_gss_wrap_stub,	__rpc_gss_wrap);
__weak_reference(__rpc_gss_unwrap_stub,	__rpc_gss_unwrap);