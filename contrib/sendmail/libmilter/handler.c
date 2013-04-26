
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

#include <sm/gen.h>
SM_RCSID("@(#)$Id: handler.c,v 8.39 2008/11/25 01:14:16 ca Exp $")

#include "libmilter.h"

#if !_FFR_WORKERS_POOL
/*
**  HANDLE_SESSION -- Handle a connected session in its own context
**
**	Parameters:
**		ctx -- context structure
**
**	Returns:
**		MI_SUCCESS/MI_FAILURE
*/

int
mi_handle_session(ctx)
	SMFICTX_PTR ctx;
{
	int ret;

	if (ctx == NULL)
		return MI_FAILURE;
	ctx->ctx_id = (sthread_t) sthread_get_id();

	/*
	**  Detach so resources are free when the thread returns.
	**  If we ever "wait" for threads, this call must be removed.
	*/

	if (pthread_detach(ctx->ctx_id) != 0)
		ret = MI_FAILURE;
	else
		ret = mi_engine(ctx);
	mi_clr_ctx(ctx);
	ctx = NULL;
	return ret;
}
#endif /* !_FFR_WORKERS_POOL */