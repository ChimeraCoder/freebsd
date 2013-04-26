
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/module.h>

#include <sys/dtrace.h>
#include "../sys/dtrace_bsd.h"


static int	dtio_unload(void);
static void	dtio_getargdesc(void *, dtrace_id_t, void *,
		    dtrace_argdesc_t *);
static void	dtio_provide(void *, dtrace_probedesc_t *);
static void	dtio_destroy(void *, dtrace_id_t, void *);
static void	dtio_enable(void *, dtrace_id_t, void *);
static void	dtio_disable(void *, dtrace_id_t, void *);
static void	dtio_load(void *);

static dtrace_pattr_t dtio_attr = {
{ DTRACE_STABILITY_STABLE, DTRACE_STABILITY_STABLE, DTRACE_CLASS_COMMON },
{ DTRACE_STABILITY_PRIVATE, DTRACE_STABILITY_PRIVATE, DTRACE_CLASS_UNKNOWN },
{ DTRACE_STABILITY_PRIVATE, DTRACE_STABILITY_PRIVATE, DTRACE_CLASS_UNKNOWN },
{ DTRACE_STABILITY_STABLE, DTRACE_STABILITY_STABLE, DTRACE_CLASS_COMMON },
{ DTRACE_STABILITY_STABLE, DTRACE_STABILITY_STABLE, DTRACE_CLASS_COMMON },
};

static char    *kernel = "kernel";

/*
 * Name strings.
 */
static char	*dtio_start_str = "start";
static char	*dtio_done_str = "done";
static char	*dtio_wait_start_str = "wait-start";
static char	*dtio_wait_done_str = "wait-done";

static dtrace_pops_t dtio_pops = {
	dtio_provide,
	NULL,
	dtio_enable,
	dtio_disable,
	NULL,
	NULL,
	dtio_getargdesc,
	NULL,
	NULL,
	dtio_destroy
};

static dtrace_provider_id_t	dtio_id;

extern uint32_t	dtio_start_id;
extern uint32_t	dtio_done_id;
extern uint32_t	dtio_wait_start_id;
extern uint32_t	dtio_wait_done_id;

static void
dtio_getargdesc(void *arg, dtrace_id_t id, void *parg,
    dtrace_argdesc_t *desc)
{
	const char *p = NULL;

	switch (desc->dtargd_ndx) {
	case 0:
		p = "struct bio *";
		break;
	case 1:
		p = "struct devstat *";
		break;
	default:
		desc->dtargd_ndx = DTRACE_ARGNONE;
	}

	if (p != NULL)
		strlcpy(desc->dtargd_native, p, sizeof(desc->dtargd_native));
}

static void
dtio_provide(void *arg, dtrace_probedesc_t *desc)
{
	if (desc != NULL)
		return;

	if (dtrace_probe_lookup(dtio_id, kernel, NULL, 
				dtio_start_str) == 0) {
		dtio_start_id = dtrace_probe_create(dtio_id, kernel, NULL, 
						   dtio_start_str, 0, NULL);
	}
	if (dtrace_probe_lookup(dtio_id, kernel, NULL, dtio_done_str) == 0) {
		dtio_done_id = dtrace_probe_create(dtio_id, kernel, NULL, 
						   dtio_done_str, 0, NULL);
	}
	if (dtrace_probe_lookup(dtio_id, kernel, NULL, 
				dtio_wait_start_str) == 0) {
		dtio_wait_start_id = dtrace_probe_create(dtio_id, kernel, 
							 NULL, 
							 dtio_wait_start_str, 
							 0, NULL);
	}
	if (dtrace_probe_lookup(dtio_id, kernel, NULL, 
				dtio_wait_done_str) == 0) {
		dtio_wait_done_id = dtrace_probe_create(dtio_id, kernel, NULL, 
						   dtio_wait_done_str, 0, NULL);
	}

}

static void
dtio_destroy(void *arg, dtrace_id_t id, void *parg)
{
}

static void
dtio_enable(void *arg, dtrace_id_t id, void *parg)
{
	if (id == dtio_start_id)
		dtrace_io_start_probe =
			(dtrace_io_start_probe_func_t)dtrace_probe;
	else if (id == dtio_done_id)
		dtrace_io_done_probe =
			(dtrace_io_done_probe_func_t)dtrace_probe;
	else if (id == dtio_wait_start_id)
		dtrace_io_wait_start_probe =
			(dtrace_io_wait_start_probe_func_t)dtrace_probe;
	else if (id == dtio_wait_done_id)
		dtrace_io_wait_done_probe =
			(dtrace_io_wait_done_probe_func_t)dtrace_probe;
	else
		printf("dtrace io provider: unknown ID\n");

}

static void
dtio_disable(void *arg, dtrace_id_t id, void *parg)
{
	if (id == dtio_start_id)
		dtrace_io_start_probe = NULL;
	else if (id == dtio_done_id)
		dtrace_io_done_probe = NULL;
	else if (id == dtio_wait_start_id)
		dtrace_io_wait_start_probe = NULL;
	else if (id == dtio_wait_done_id)
		dtrace_io_wait_done_probe = NULL;
	else 
		printf("dtrace io provider: unknown ID\n");
	
}

static void
dtio_load(void *dummy)
{
	if (dtrace_register("io", &dtio_attr, DTRACE_PRIV_USER, NULL, 
			    &dtio_pops, NULL, &dtio_id) != 0)
		return;
}


static int
dtio_unload()
{
	dtrace_io_start_probe = NULL;
	dtrace_io_done_probe = NULL;
	dtrace_io_wait_start_probe = NULL;
	dtrace_io_wait_done_probe = NULL;

	return (dtrace_unregister(dtio_id));
}

static int
dtio_modevent(module_t mod __unused, int type, void *data __unused)
{
	int error = 0;

	switch (type) {
	case MOD_LOAD:
		break;

	case MOD_UNLOAD:
		break;

	case MOD_SHUTDOWN:
		break;

	default:
		error = EOPNOTSUPP;
		break;
	}

	return (error);
}

SYSINIT(dtio_load, SI_SUB_DTRACE_PROVIDER, SI_ORDER_ANY,
    dtio_load, NULL);
SYSUNINIT(dtio_unload, SI_SUB_DTRACE_PROVIDER, SI_ORDER_ANY,
    dtio_unload, NULL);

DEV_MODULE(dtio, dtio_modevent, NULL);
MODULE_VERSION(dtio, 1);
MODULE_DEPEND(dtio, dtrace, 1, 1, 1);
MODULE_DEPEND(dtio, opensolaris, 1, 1, 1);