
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

static int current_concurrency = 0;

__weak_reference(_pthread_getconcurrency, pthread_getconcurrency);
__weak_reference(_pthread_setconcurrency, pthread_setconcurrency);

int
_pthread_getconcurrency(void)
{
	return current_concurrency;
}

int
_pthread_setconcurrency(int new_level)
{
	int ret;

	if (new_level < 0) {
		ret = EINVAL;
	} else {
		current_concurrency = new_level;
		ret = 0;
	}
	return (ret);
}