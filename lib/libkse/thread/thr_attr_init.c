
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
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include "un-namespace.h"
#include "thr_private.h"

__weak_reference(_pthread_attr_init, pthread_attr_init);

int
_pthread_attr_init(pthread_attr_t *attr)
{
	int	ret;
	pthread_attr_t	pattr;

	/* Allocate memory for the attribute object: */
	if ((pattr = (pthread_attr_t) malloc(sizeof(struct pthread_attr))) == NULL)
		/* Insufficient memory: */
		ret = ENOMEM;
	else {
		/* Initialise the attribute object with the defaults: */
		memcpy(pattr, &_pthread_attr_default,
		    sizeof(struct pthread_attr));
		pattr->guardsize_attr = _thr_guard_default;
		pattr->stacksize_attr = _thr_stack_default;

		/* Return a pointer to the attribute object: */
		*attr = pattr;
		ret = 0;
	}
	return(ret);
}