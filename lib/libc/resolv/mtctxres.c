
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
 * Initialize the TSD key. By doing this at library load time, we're
 * implicitly running without interference from other threads, so there's
 * no need for locking.
 */static void
_mtctxres_init(void) {
	int pthread_keycreate_ret;

	pthread_keycreate_ret = pthread_key_create(&key, __res_destroy_ctx);
	if (pthread_keycreate_ret == 0)
		mt_key_initialized = 1;
}
#endif

#ifndef _LIBC
/*
 * To support binaries that used the private MT-safe interface in
 * Solaris 8, we still need to provide the __res_enable_mt()
 * and __res_disable_mt() entry points. They're do-nothing routines.
 */
int
__res_enable_mt(void) {
	return (-1);
}

int
__res_disable_mt(void) {
	return (0);
}
#endif

#ifdef DO_PTHREADS
static int
__res_init_ctx(void) {

	mtctxres_t	*mt;
	int		ret;


	if (pthread_getspecific(key) != 0) {
		/* Already exists */
		return (0);
	}

	if ((mt = malloc(sizeof (mtctxres_t))) == 0) {
		errno = ENOMEM;
		return (-1);
	}

	memset(mt, 0, sizeof (mtctxres_t));

	if ((ret = pthread_setspecific(key, mt)) != 0) {
		free(mt);
		errno = ret;
		return (-1);
	}

	return (0);
}

static void
__res_destroy_ctx(void *value) {

	mtctxres_t	*mt = (mtctxres_t *)value;

	if (mt != 0)
		free(mt);
}
#endif

mtctxres_t *
___mtctxres(void) {
#ifdef DO_PTHREADS
	mtctxres_t	*mt;

#ifdef _LIBC
	if (pthread_main_np() != 0)
		return (&sharedctx);
#endif

	/*
	 * This if clause should only be executed if we are linking
	 * statically.  When linked dynamically _mtctxres_init() should
	 * be called at binding time due the #pragma above.
	 */
	if (!mt_key_initialized) {
		static pthread_mutex_t keylock = PTHREAD_MUTEX_INITIALIZER;
                if (pthread_mutex_lock(&keylock) == 0) {
			_mtctxres_init();
			(void) pthread_mutex_unlock(&keylock);
		}
	}

	/*
	 * If we have already been called in this thread return the existing
	 * context.  Otherwise recreat a new context and return it.  If
	 * that fails return a global context.
	 */
	if (mt_key_initialized) {
		if (((mt = pthread_getspecific(key)) != 0) ||
		    (__res_init_ctx() == 0 &&
		     (mt = pthread_getspecific(key)) != 0)) {
			return (mt);
		}
	}
#endif
	return (&sharedctx);
}