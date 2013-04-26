
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

__weak_reference(_pthread_attr_getstack, pthread_attr_getstack);

int
_pthread_attr_getstack(const pthread_attr_t * __restrict attr,
                        void ** __restrict stackaddr,
                        size_t * __restrict stacksize)
{
	int     ret;

	/* Check for invalid arguments: */
	if (attr == NULL || *attr == NULL || stackaddr == NULL
	    || stacksize == NULL )
		ret = EINVAL;
	else {
		/* Return the stack address and size */
		*stackaddr = (*attr)->stackaddr_attr;
		*stacksize = (*attr)->stacksize_attr;
		ret = 0;
	}
	return(ret);
}