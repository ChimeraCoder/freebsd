
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

#include "namespace.h"
#include <errno.h>
#include <pthread.h>
#include <pthread_np.h>
#include "un-namespace.h"
#include "thr_private.h"

__weak_reference(_pthread_switch_add_np, pthread_switch_add_np);
__weak_reference(_pthread_switch_delete_np, pthread_switch_delete_np);

int
_pthread_switch_add_np(pthread_switch_routine_t routine __unused)
{
	return (ENOTSUP);
}

int
_pthread_switch_delete_np(pthread_switch_routine_t routine __unused)
{
	return (ENOTSUP);
}