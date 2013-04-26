
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
#include <sys/time.h>
#include "un-namespace.h"

#include "thr_private.h"

__weak_reference(_pthread_getcpuclockid, pthread_getcpuclockid);

int
_pthread_getcpuclockid(pthread_t pthread, clockid_t *clock_id)
{

	if (pthread == NULL)
		return (EINVAL);

	if (clock_getcpuclockid2(TID(pthread), CPUCLOCK_WHICH_TID, clock_id))
		return (errno);
	return (0);
}