
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
#include <pthread.h>
#include <pthread_np.h>
#include "un-namespace.h"
#include "thr_private.h"

__weak_reference(_pthread_main_np, pthread_main_np);

/*
 * Provide the equivelant to Solaris thr_main() function
 */
int
_pthread_main_np()
{

	if (!_thr_initial)
		return (-1);
	else
		return (_pthread_equal(_pthread_self(), _thr_initial) ? 1 : 0);
}