
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

/*
 * POSIX stdio FILE locking functions. These assume that the locking
 * is only required at FILE structure level, not at file descriptor
 * level too.
 *
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "un-namespace.h"

#include "local.h"


/*
 * Weak symbols for externally visible functions in this file:
 */
__weak_reference(_flockfile, flockfile);
__weak_reference(_flockfile_debug_stub, _flockfile_debug);
__weak_reference(_ftrylockfile, ftrylockfile);
__weak_reference(_funlockfile, funlockfile);

void
_flockfile(FILE *fp)
{
	pthread_t curthread = _pthread_self();

	if (fp->_fl_owner == curthread)
		fp->_fl_count++;
	else {
		/*
		 * Make sure this mutex is treated as a private
		 * internal mutex:
		 */
		_pthread_mutex_lock(&fp->_fl_mutex);
		fp->_fl_owner = curthread;
		fp->_fl_count = 1;
	}
}

/*
 * This can be overriden by the threads library if it is linked in.
 */
void
_flockfile_debug_stub(FILE *fp, char *fname, int lineno)
{
	_flockfile(fp);
}

int
_ftrylockfile(FILE *fp)
{
	pthread_t curthread = _pthread_self();
	int	ret = 0;

	if (fp->_fl_owner == curthread)
		fp->_fl_count++;
	/*
	 * Make sure this mutex is treated as a private
	 * internal mutex:
	 */
	else if (_pthread_mutex_trylock(&fp->_fl_mutex) == 0) {
		fp->_fl_owner = curthread;
		fp->_fl_count = 1;
	}
	else
		ret = -1;
	return (ret);
}

void 
_funlockfile(FILE *fp)
{
	pthread_t	curthread = _pthread_self();

	/*
	 * Check if this file is owned by the current thread:
	 */
	if (fp->_fl_owner == curthread) {
		/*
		 * Check if this thread has locked the FILE
		 * more than once:
		 */
		if (fp->_fl_count > 1)
			/*
			 * Decrement the count of the number of
			 * times the running thread has locked this
			 * file:
			 */
			fp->_fl_count--;
		else {
			/*
			 * The running thread will release the
			 * lock now:
			 */
			fp->_fl_count = 0;
			fp->_fl_owner = NULL;
			_pthread_mutex_unlock(&fp->_fl_mutex);
		}
	}
}