
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
 * Counting semaphores.
 *
 * Priority propagation will not generally raise the priority of semaphore
 * "owners" (a misnomer in the context of semaphores), so should not be relied
 * upon in combination with semaphores.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/ktr.h>
#include <sys/condvar.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/sema.h>

void
sema_init(struct sema *sema, int value, const char *description)
{

	KASSERT((value >= 0), ("%s(): negative value\n", __func__));

	bzero(sema, sizeof(*sema));
	mtx_init(&sema->sema_mtx, description, "sema backing lock",
	    MTX_DEF | MTX_NOWITNESS | MTX_QUIET);
	cv_init(&sema->sema_cv, description);
	sema->sema_value = value;

	CTR4(KTR_LOCK, "%s(%p, %d, \"%s\")", __func__, sema, value, description);
}

void
sema_destroy(struct sema *sema)
{

	CTR3(KTR_LOCK, "%s(%p) \"%s\"", __func__, sema,
	    cv_wmesg(&sema->sema_cv));

	KASSERT((sema->sema_waiters == 0), ("%s(): waiters\n", __func__));

	mtx_destroy(&sema->sema_mtx);
	cv_destroy(&sema->sema_cv);
}

void
_sema_post(struct sema *sema, const char *file, int line)
{

	mtx_lock(&sema->sema_mtx);
	sema->sema_value++;
	if (sema->sema_waiters && sema->sema_value > 0)
		cv_signal(&sema->sema_cv);

	CTR6(KTR_LOCK, "%s(%p) \"%s\" v = %d at %s:%d", __func__, sema,
	    cv_wmesg(&sema->sema_cv), sema->sema_value, file, line);

	mtx_unlock(&sema->sema_mtx);
}

void
_sema_wait(struct sema *sema, const char *file, int line)
{

	mtx_lock(&sema->sema_mtx);
	while (sema->sema_value == 0) {
		sema->sema_waiters++;
		cv_wait(&sema->sema_cv, &sema->sema_mtx);
		sema->sema_waiters--;
	}
	sema->sema_value--;

	CTR6(KTR_LOCK, "%s(%p) \"%s\" v = %d at %s:%d", __func__, sema,
	    cv_wmesg(&sema->sema_cv), sema->sema_value, file, line);

	mtx_unlock(&sema->sema_mtx);
}

int
_sema_timedwait(struct sema *sema, int timo, const char *file, int line)
{
	int error;

	mtx_lock(&sema->sema_mtx);

	/*
	 * A spurious wakeup will cause the timeout interval to start over.
	 * This isn't a big deal as long as spurious wakeups don't occur
	 * continuously, since the timeout period is merely a lower bound on how
	 * long to wait.
	 */
	for (error = 0; sema->sema_value == 0 && error == 0;) {
		sema->sema_waiters++;
		error = cv_timedwait(&sema->sema_cv, &sema->sema_mtx, timo);
		sema->sema_waiters--;
	}
	if (sema->sema_value > 0) {
		/* Success. */
		sema->sema_value--;
		error = 0;

		CTR6(KTR_LOCK, "%s(%p) \"%s\" v = %d at %s:%d", __func__, sema,
		    cv_wmesg(&sema->sema_cv), sema->sema_value, file, line);
	} else {
		CTR5(KTR_LOCK, "%s(%p) \"%s\" fail at %s:%d", __func__, sema,
		    cv_wmesg(&sema->sema_cv), file, line);
	}

	mtx_unlock(&sema->sema_mtx);
	return (error);
}

int
_sema_trywait(struct sema *sema, const char *file, int line)
{
	int ret;

	mtx_lock(&sema->sema_mtx);

	if (sema->sema_value > 0) {
		/* Success. */
		sema->sema_value--;
		ret = 1;

		CTR6(KTR_LOCK, "%s(%p) \"%s\" v = %d at %s:%d", __func__, sema,
		    cv_wmesg(&sema->sema_cv), sema->sema_value, file, line);
	} else {
		ret = 0;

		CTR5(KTR_LOCK, "%s(%p) \"%s\" fail at %s:%d", __func__, sema,
		    cv_wmesg(&sema->sema_cv), file, line);
	}

	mtx_unlock(&sema->sema_mtx);
	return (ret);
}

int
sema_value(struct sema *sema)
{
	int ret;

	mtx_lock(&sema->sema_mtx);
	ret = sema->sema_value;
	mtx_unlock(&sema->sema_mtx);
	return (ret);
}