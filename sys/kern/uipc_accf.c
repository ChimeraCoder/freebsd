
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

#define ACCEPT_FILTER_MOD

#include "opt_param.h"
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/domain.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/protosw.h>
#include <sys/sysctl.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/queue.h>

static struct mtx accept_filter_mtx;
MTX_SYSINIT(accept_filter, &accept_filter_mtx, "accept_filter_mtx",
	MTX_DEF);
#define	ACCEPT_FILTER_LOCK()	mtx_lock(&accept_filter_mtx)
#define	ACCEPT_FILTER_UNLOCK()	mtx_unlock(&accept_filter_mtx)

static SLIST_HEAD(, accept_filter) accept_filtlsthd =
	SLIST_HEAD_INITIALIZER(accept_filtlsthd);

MALLOC_DEFINE(M_ACCF, "accf", "accept filter data");

static int unloadable = 0;

SYSCTL_DECL(_net_inet);	/* XXX: some header should do this for me */
SYSCTL_NODE(_net_inet, OID_AUTO, accf, CTLFLAG_RW, 0, "Accept filters");
SYSCTL_INT(_net_inet_accf, OID_AUTO, unloadable, CTLFLAG_RW, &unloadable, 0,
	"Allow unload of accept filters (not recommended)");

/*
 * Must be passed a malloc'd structure so we don't explode if the kld is
 * unloaded, we leak the struct on deallocation to deal with this, but if a
 * filter is loaded with the same name as a leaked one we re-use the entry.
 */
int
accept_filt_add(struct accept_filter *filt)
{
	struct accept_filter *p;

	ACCEPT_FILTER_LOCK();
	SLIST_FOREACH(p, &accept_filtlsthd, accf_next)
		if (strcmp(p->accf_name, filt->accf_name) == 0)  {
			if (p->accf_callback != NULL) {
				ACCEPT_FILTER_UNLOCK();
				return (EEXIST);
			} else {
				p->accf_callback = filt->accf_callback;
				ACCEPT_FILTER_UNLOCK();
				free(filt, M_ACCF);
				return (0);
			}
		}
				
	if (p == NULL)
		SLIST_INSERT_HEAD(&accept_filtlsthd, filt, accf_next);
	ACCEPT_FILTER_UNLOCK();
	return (0);
}

int
accept_filt_del(char *name)
{
	struct accept_filter *p;

	p = accept_filt_get(name);
	if (p == NULL)
		return (ENOENT);

	p->accf_callback = NULL;
	return (0);
}

struct accept_filter *
accept_filt_get(char *name)
{
	struct accept_filter *p;

	ACCEPT_FILTER_LOCK();
	SLIST_FOREACH(p, &accept_filtlsthd, accf_next)
		if (strcmp(p->accf_name, name) == 0)
			break;
	ACCEPT_FILTER_UNLOCK();

	return (p);
}

int
accept_filt_generic_mod_event(module_t mod, int event, void *data)
{
	struct accept_filter *p;
	struct accept_filter *accfp = (struct accept_filter *) data;
	int error;

	switch (event) {
	case MOD_LOAD:
		p = malloc(sizeof(*p), M_ACCF,
		    M_WAITOK);
		bcopy(accfp, p, sizeof(*p));
		error = accept_filt_add(p);
		break;

	case MOD_UNLOAD:
		/*
		 * Do not support unloading yet. we don't keep track of
		 * refcounts and unloading an accept filter callback and then
		 * having it called is a bad thing.  A simple fix would be to
		 * track the refcount in the struct accept_filter.
		 */
		if (unloadable != 0) {
			error = accept_filt_del(accfp->accf_name);
		} else
			error = EOPNOTSUPP;
		break;

	case MOD_SHUTDOWN:
		error = 0;
		break;

	default:
		error = EOPNOTSUPP;
		break;
	}

	return (error);
}

int
do_getopt_accept_filter(struct socket *so, struct sockopt *sopt)
{
	struct accept_filter_arg *afap;
	int error;

	error = 0;
	afap = malloc(sizeof(*afap), M_TEMP,
	    M_WAITOK | M_ZERO);
	SOCK_LOCK(so);
	if ((so->so_options & SO_ACCEPTCONN) == 0) {
		error = EINVAL;
		goto out;
	}
	if ((so->so_options & SO_ACCEPTFILTER) == 0) {
		error = EINVAL;
		goto out;
	}
	strcpy(afap->af_name, so->so_accf->so_accept_filter->accf_name);
	if (so->so_accf->so_accept_filter_str != NULL)
		strcpy(afap->af_arg, so->so_accf->so_accept_filter_str);
out:
	SOCK_UNLOCK(so);
	if (error == 0)
		error = sooptcopyout(sopt, afap, sizeof(*afap));
	free(afap, M_TEMP);
	return (error);
}

int
do_setopt_accept_filter(struct socket *so, struct sockopt *sopt)
{
	struct accept_filter_arg *afap;
	struct accept_filter *afp;
	struct so_accf *newaf;
	int error = 0;

	/*
	 * Handle the simple delete case first.
	 */
	if (sopt == NULL || sopt->sopt_val == NULL) {
		SOCK_LOCK(so);
		if ((so->so_options & SO_ACCEPTCONN) == 0) {
			SOCK_UNLOCK(so);
			return (EINVAL);
		}
		if (so->so_accf != NULL) {
			struct so_accf *af = so->so_accf;
			if (af->so_accept_filter != NULL &&
				af->so_accept_filter->accf_destroy != NULL) {
				af->so_accept_filter->accf_destroy(so);
			}
			if (af->so_accept_filter_str != NULL)
				free(af->so_accept_filter_str, M_ACCF);
			free(af, M_ACCF);
			so->so_accf = NULL;
		}
		so->so_options &= ~SO_ACCEPTFILTER;
		SOCK_UNLOCK(so);
		return (0);
	}

	/*
	 * Pre-allocate any memory we may need later to avoid blocking at
	 * untimely moments.  This does not optimize for invalid arguments.
	 */
	afap = malloc(sizeof(*afap), M_TEMP,
	    M_WAITOK);
	error = sooptcopyin(sopt, afap, sizeof *afap, sizeof *afap);
	afap->af_name[sizeof(afap->af_name)-1] = '\0';
	afap->af_arg[sizeof(afap->af_arg)-1] = '\0';
	if (error) {
		free(afap, M_TEMP);
		return (error);
	}
	afp = accept_filt_get(afap->af_name);
	if (afp == NULL) {
		free(afap, M_TEMP);
		return (ENOENT);
	}
	/*
	 * Allocate the new accept filter instance storage.  We may
	 * have to free it again later if we fail to attach it.  If
	 * attached properly, 'newaf' is NULLed to avoid a free()
	 * while in use.
	 */
	newaf = malloc(sizeof(*newaf), M_ACCF, M_WAITOK |
	    M_ZERO);
	if (afp->accf_create != NULL && afap->af_name[0] != '\0') {
		int len = strlen(afap->af_name) + 1;
		newaf->so_accept_filter_str = malloc(len, M_ACCF,
		    M_WAITOK);
		strcpy(newaf->so_accept_filter_str, afap->af_name);
	}

	/*
	 * Require a listen socket; don't try to replace an existing filter
	 * without first removing it.
	 */
	SOCK_LOCK(so);
	if (((so->so_options & SO_ACCEPTCONN) == 0) ||
	    (so->so_accf != NULL)) {
		error = EINVAL;
		goto out;
	}

	/*
	 * Invoke the accf_create() method of the filter if required.  The
	 * socket mutex is held over this call, so create methods for filters
	 * can't block.
	 */
	if (afp->accf_create != NULL) {
		newaf->so_accept_filter_arg =
		    afp->accf_create(so, afap->af_arg);
		if (newaf->so_accept_filter_arg == NULL) {
			error = EINVAL;
			goto out;
		}
	}
	newaf->so_accept_filter = afp;
	so->so_accf = newaf;
	so->so_options |= SO_ACCEPTFILTER;
	newaf = NULL;
out:
	SOCK_UNLOCK(so);
	if (newaf != NULL) {
		if (newaf->so_accept_filter_str != NULL)
			free(newaf->so_accept_filter_str, M_ACCF);
		free(newaf, M_ACCF);
	}
	if (afap != NULL)
		free(afap, M_TEMP);
	return (error);
}