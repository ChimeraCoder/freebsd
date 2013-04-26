
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
#include <sys/kobj.h>
#include <sys/malloc.h>
#include <rpc/rpc.h>
#include <rpc/rpcsec_gss.h>

#include "rpcsec_gss_int.h"

static rpc_gss_error_t _rpc_gss_error;

void
_rpc_gss_set_error(int rpc_gss_error, int system_error)
{

	_rpc_gss_error.rpc_gss_error = rpc_gss_error;
	_rpc_gss_error.system_error = system_error;
}

void
rpc_gss_get_error(rpc_gss_error_t *error)
{

	*error = _rpc_gss_error;
}