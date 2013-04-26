
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

__weak_reference(_pthread_multi_np, pthread_multi_np);

int
_pthread_multi_np()
{

	/* Return to multi-threaded scheduling mode: */
	/*
	 * XXX - Do we want to do this?
	 * __is_threaded = 1;
	 */
	_pthread_resume_all_np();
	return (0);
}