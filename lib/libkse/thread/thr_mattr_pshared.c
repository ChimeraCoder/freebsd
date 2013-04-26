
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
#include "un-namespace.h"
#include "thr_private.h"

int _pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr,
	int *pshared);
int _pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared);


__weak_reference(_pthread_mutexattr_getpshared, pthread_mutexattr_getpshared);
__weak_reference(_pthread_mutexattr_setpshared, pthread_mutexattr_setpshared);

int
_pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr,
	int *pshared)
{
	if (attr == NULL || *attr == NULL)
		return (EINVAL);

	pshared = PTHREAD_PROCESS_PRIVATE;
	return (0);
}

int
_pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared)
{
	if (attr == NULL || *attr == NULL)
		return (EINVAL);

	if  (pshared != PTHREAD_PROCESS_PRIVATE)
		return (EINVAL);
	return (0);
}