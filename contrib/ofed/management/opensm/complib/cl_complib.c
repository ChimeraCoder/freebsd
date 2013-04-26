
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

#if HAVE_CONFIG_H
#  include <config.h>
#endif				/* HAVE_CONFIG_H */

#include <complib/cl_types.h>
#include <complib/cl_debug.h>
#include <complib/cl_spinlock.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/*
 *  Prototypes
 */

extern
 cl_status_t __cl_timer_prov_create(void);

extern
void __cl_timer_prov_destroy(void);

cl_spinlock_t cl_atomic_spinlock;

void complib_init(void)
{
	cl_status_t status = CL_SUCCESS;

	status = cl_spinlock_init(&cl_atomic_spinlock);
	if (status != CL_SUCCESS)
		goto _error;

	status = __cl_timer_prov_create();
	if (status != CL_SUCCESS)
		goto _error;
	return;

_error:
	cl_msg_out("__init: failed to create complib (%s)\n",
		   CL_STATUS_MSG(status));
	exit(1);
}

void complib_exit(void)
{
	__cl_timer_prov_destroy();
	cl_spinlock_destroy(&cl_atomic_spinlock);
}

boolean_t cl_is_debug(void)
{
#if defined( _DEBUG_ )
	return TRUE;
#else
	return FALSE;
#endif				/* defined( _DEBUG_ ) */
}