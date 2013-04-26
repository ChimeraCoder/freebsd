
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

#include <pthread.h>
#include <pthread_np.h>
#include <stdio.h>
#include <stdlib.h>

static void *
thread(void *arg)
{
	pthread_mutex_t *mtx = arg;

	if (pthread_mutex_isowned_np(mtx) != 0) {
		printf("pthread_mutex_isowned_np() returned non-zero\n"
		    "for a mutex held by another thread\n");
		exit(1);
	}
	return (NULL);
}

int
main(int argc, char *argv[])
{
	pthread_t thr;
	pthread_mutex_t mtx;

	pthread_mutex_init(&mtx, NULL);
	if (pthread_mutex_isowned_np(&mtx) != 0) {
		printf("pthread_mutex_isowned_np() returned non-zero\n"
		    "for a mutex that is not held\n");
		exit(1);
	}
	pthread_mutex_lock(&mtx);
	if (pthread_mutex_isowned_np(&mtx) == 0) {
		printf("pthread_mutex_isowned_np() returned zero\n"
		    "for a mutex we hold ourselves\n");
		exit(1);
	}
	pthread_create(&thr, NULL, thread, &mtx);
	pthread_join(thr, NULL);
	pthread_mutex_unlock(&mtx);
	if (pthread_mutex_isowned_np(&mtx) != 0) {
		printf("pthread_mutex_isowned_np() returned non-zero\n"
		    "for a mutex that is not held\n");
		exit(1);
	}

	printf("OK\n");
	exit(0);
}