
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
#include "un-namespace.h"
#include "thr_private.h"

__weak_reference(_pthread_attr_getdetachstate, pthread_attr_getdetachstate);

int
_pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate)
{
	int	ret;

	/* Check for invalid arguments: */
	if (attr == NULL || *attr == NULL || detachstate == NULL)
		ret = EINVAL;
	else {
		/* Check if the detached flag is set: */
		if ((*attr)->flags & PTHREAD_DETACHED)
			/* Return detached: */
			*detachstate = PTHREAD_CREATE_DETACHED;
		else
			/* Return joinable: */
			*detachstate = PTHREAD_CREATE_JOINABLE;
		ret = 0;
	}
	return(ret);
}