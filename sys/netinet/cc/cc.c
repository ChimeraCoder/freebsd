
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
 * This software was first released in 2007 by James Healy and Lawrence Stewart
 * whilst working on the NewTCP research project at Swinburne University of
 * Technology's Centre for Advanced Internet Architectures, Melbourne,
 * Australia, which was made possible in part by a grant from the Cisco
 * University Research Program Fund at Community Foundation Silicon Valley.
 * More details are available at:
 *   http://caia.swin.edu.au/urp/newtcp/
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/libkern.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/queue.h>
#include <sys/rwlock.h>
#include <sys/sbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/sysctl.h>

#include <net/if.h>
#include <net/if_var.h>

#include <netinet/cc.h>
#include <netinet/in.h>
#include <netinet/in_pcb.h>
#include <netinet/tcp_var.h>

#include <netinet/cc/cc_module.h>

/*
 * List of available cc algorithms on the current system. First element
 * is used as the system default CC algorithm.
 */
struct cc_head cc_list = STAILQ_HEAD_INITIALIZER(cc_list);

/* Protects the cc_list TAILQ. */
struct rwlock cc_list_lock;

VNET_DEFINE(struct cc_algo *, default_cc_ptr) = &newreno_cc_algo;

/*
 * Sysctl handler to show and change the default CC algorithm.
 */
static int
cc_default_algo(SYSCTL_HANDLER_ARGS)
{
	char default_cc[TCP_CA_NAME_MAX];
	struct cc_algo *funcs;
	int err, found;

	err = found = 0;

	if (req->newptr == NULL) {
		/* Just print the current default. */
		CC_LIST_RLOCK();
		strlcpy(default_cc, CC_DEFAULT()->name, TCP_CA_NAME_MAX);
		CC_LIST_RUNLOCK();
		err = sysctl_handle_string(oidp, default_cc, 1, req);
	} else {
		/* Find algo with specified name and set it to default. */
		CC_LIST_RLOCK();
		STAILQ_FOREACH(funcs, &cc_list, entries) {
			if (strncmp((char *)req->newptr, funcs->name,
			    TCP_CA_NAME_MAX) == 0) {
				found = 1;
				V_default_cc_ptr = funcs;
			}
		}
		CC_LIST_RUNLOCK();

		if (!found)
			err = ESRCH;
	}

	return (err);
}

/*
 * Sysctl handler to display the list of available CC algorithms.
 */
static int
cc_list_available(SYSCTL_HANDLER_ARGS)
{
	struct cc_algo *algo;
	struct sbuf *s;
	int err, first, nalgos;

	err = nalgos = 0;
	first = 1;

	CC_LIST_RLOCK();
	STAILQ_FOREACH(algo, &cc_list, entries) {
		nalgos++;
	}
	CC_LIST_RUNLOCK();

	s = sbuf_new(NULL, NULL, nalgos * TCP_CA_NAME_MAX, SBUF_FIXEDLEN);

	if (s == NULL)
		return (ENOMEM);

	/*
	 * It is theoretically possible for the CC list to have grown in size
	 * since the call to sbuf_new() and therefore for the sbuf to be too
	 * small. If this were to happen (incredibly unlikely), the sbuf will
	 * reach an overflow condition, sbuf_printf() will return an error and
	 * the sysctl will fail gracefully.
	 */
	CC_LIST_RLOCK();
	STAILQ_FOREACH(algo, &cc_list, entries) {
		err = sbuf_printf(s, first ? "%s" : ", %s", algo->name);
		if (err) {
			/* Sbuf overflow condition. */
			err = EOVERFLOW;
			break;
		}
		first = 0;
	}
	CC_LIST_RUNLOCK();

	if (!err) {
		sbuf_finish(s);
		err = sysctl_handle_string(oidp, sbuf_data(s), 1, req);
	}

	sbuf_delete(s);
	return (err);
}

/*
 * Reset the default CC algo to NewReno for any netstack which is using the algo
 * that is about to go away as its default.
 */
static void
cc_checkreset_default(struct cc_algo *remove_cc)
{
	VNET_ITERATOR_DECL(vnet_iter);

	CC_LIST_LOCK_ASSERT();

	VNET_LIST_RLOCK_NOSLEEP();
	VNET_FOREACH(vnet_iter) {
		CURVNET_SET(vnet_iter);
		if (strncmp(CC_DEFAULT()->name, remove_cc->name,
		    TCP_CA_NAME_MAX) == 0)
			V_default_cc_ptr = &newreno_cc_algo;
		CURVNET_RESTORE();
	}
	VNET_LIST_RUNLOCK_NOSLEEP();
}

/*
 * Initialise CC subsystem on system boot.
 */
static void
cc_init(void)
{
	CC_LIST_LOCK_INIT();
	STAILQ_INIT(&cc_list);
}

/*
 * Returns non-zero on success, 0 on failure.
 */
int
cc_deregister_algo(struct cc_algo *remove_cc)
{
	struct cc_algo *funcs, *tmpfuncs;
	int err;

	err = ENOENT;

	/* Never allow newreno to be deregistered. */
	if (&newreno_cc_algo == remove_cc)
		return (EPERM);

	/* Remove algo from cc_list so that new connections can't use it. */
	CC_LIST_WLOCK();
	STAILQ_FOREACH_SAFE(funcs, &cc_list, entries, tmpfuncs) {
		if (funcs == remove_cc) {
			cc_checkreset_default(remove_cc);
			STAILQ_REMOVE(&cc_list, funcs, cc_algo, entries);
			err = 0;
			break;
		}
	}
	CC_LIST_WUNLOCK();

	if (!err)
		/*
		 * XXXLAS:
		 * - We may need to handle non-zero return values in future.
		 * - If we add CC framework support for protocols other than
		 *   TCP, we may want a more generic way to handle this step.
		 */
		tcp_ccalgounload(remove_cc);

	return (err);
}

/*
 * Returns 0 on success, non-zero on failure.
 */
int
cc_register_algo(struct cc_algo *add_cc)
{
	struct cc_algo *funcs;
	int err;

	err = 0;

	/*
	 * Iterate over list of registered CC algorithms and make sure
	 * we're not trying to add a duplicate.
	 */
	CC_LIST_WLOCK();
	STAILQ_FOREACH(funcs, &cc_list, entries) {
		if (funcs == add_cc || strncmp(funcs->name, add_cc->name,
		    TCP_CA_NAME_MAX) == 0)
			err = EEXIST;
	}

	if (!err)
		STAILQ_INSERT_TAIL(&cc_list, add_cc, entries);

	CC_LIST_WUNLOCK();

	return (err);
}

/*
 * Handles kld related events. Returns 0 on success, non-zero on failure.
 */
int
cc_modevent(module_t mod, int event_type, void *data)
{
	struct cc_algo *algo;
	int err;

	err = 0;
	algo = (struct cc_algo *)data;

	switch(event_type) {
	case MOD_LOAD:
		if (algo->mod_init != NULL)
			err = algo->mod_init();
		if (!err)
			err = cc_register_algo(algo);
		break;

	case MOD_QUIESCE:
	case MOD_SHUTDOWN:
	case MOD_UNLOAD:
		err = cc_deregister_algo(algo);
		if (!err && algo->mod_destroy != NULL)
			algo->mod_destroy();
		if (err == ENOENT)
			err = 0;
		break;

	default:
		err = EINVAL;
		break;
	}

	return (err);
}

SYSINIT(cc, SI_SUB_PROTO_IFATTACHDOMAIN, SI_ORDER_FIRST, cc_init, NULL);

/* Declare sysctl tree and populate it. */
SYSCTL_NODE(_net_inet_tcp, OID_AUTO, cc, CTLFLAG_RW, NULL,
    "congestion control related settings");

SYSCTL_VNET_PROC(_net_inet_tcp_cc, OID_AUTO, algorithm, CTLTYPE_STRING|CTLFLAG_RW,
    NULL, 0, cc_default_algo, "A", "default congestion control algorithm");

SYSCTL_PROC(_net_inet_tcp_cc, OID_AUTO, available, CTLTYPE_STRING|CTLFLAG_RD,
    NULL, 0, cc_list_available, "A",
    "list available congestion control algorithms");