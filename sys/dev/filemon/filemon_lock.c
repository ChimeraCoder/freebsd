
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

static void
filemon_filemon_lock(struct filemon *filemon)
{
	mtx_lock(&filemon->mtx);

	while (filemon->locker != NULL && filemon->locker != curthread)
		cv_wait(&filemon->cv, &filemon->mtx);

	filemon->locker = curthread;

	mtx_unlock(&filemon->mtx);
}

static void
filemon_filemon_unlock(struct filemon *filemon)
{
	mtx_lock(&filemon->mtx);

	if (filemon->locker == curthread)
		filemon->locker = NULL;

	/* Wake up threads waiting. */
	cv_broadcast(&filemon->cv);

	mtx_unlock(&filemon->mtx);
}

static void
filemon_lock_read(void)
{
	mtx_lock(&access_mtx);

	while (access_owner != NULL || access_requester != NULL)
		cv_wait(&access_cv, &access_mtx);

	n_readers++;

	/* Wake up threads waiting. */
	cv_broadcast(&access_cv);

	mtx_unlock(&access_mtx);
}

static void
filemon_unlock_read(void)
{
	mtx_lock(&access_mtx);

	if (n_readers > 0)
		n_readers--;

	/* Wake up a thread waiting. */
	cv_broadcast(&access_cv);

	mtx_unlock(&access_mtx);
}

static void
filemon_lock_write(void)
{
	mtx_lock(&access_mtx);

	while (access_owner != curthread) {
		if (access_owner == NULL &&
		    (access_requester == NULL ||
		    access_requester == curthread)) {
			access_owner = curthread;
			access_requester = NULL;
		} else {
			if (access_requester == NULL)
				access_requester = curthread;

			cv_wait(&access_cv, &access_mtx);
		}
	}

	mtx_unlock(&access_mtx);
}

static void
filemon_unlock_write(void)
{
	mtx_lock(&access_mtx);

	/* Sanity check that the current thread actually has the write lock. */
	if (access_owner == curthread)
		access_owner = NULL;

	/* Wake up a thread waiting. */
	cv_broadcast(&access_cv);

	mtx_unlock(&access_mtx);
}