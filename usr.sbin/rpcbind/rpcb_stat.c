
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
 * rpcb_stat.c
 * Allows for gathering of statistics
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <netconfig.h>
#include <rpc/rpc.h>
#include <rpc/rpcb_prot.h>
#include <sys/stat.h>
#ifdef PORTMAP
#include <rpc/pmap_prot.h>
#endif
#include <stdlib.h>
#include <string.h>
#include "rpcbind.h"

static rpcb_stat_byvers inf;

void
rpcbs_init(void)
{

}

void
rpcbs_procinfo(rpcvers_t rtype, rpcproc_t proc)
{
	switch (rtype + 2) {
#ifdef PORTMAP
	case PMAPVERS:		/* version 2 */
		if (proc > rpcb_highproc_2)
			return;
		break;
#endif
	case RPCBVERS:		/* version 3 */
		if (proc > rpcb_highproc_3)
			return;
		break;
	case RPCBVERS4:		/* version 4 */
		if (proc > rpcb_highproc_4)
			return;
		break;
	default: return;
	}
	inf[rtype].info[proc]++;
	return;
}

void
rpcbs_set(rpcvers_t rtype, bool_t success)
{
	if ((rtype >= RPCBVERS_STAT) || (success == FALSE))
		return;
	inf[rtype].setinfo++;
	return;
}

void
rpcbs_unset(rpcvers_t rtype, bool_t success)
{
	if ((rtype >= RPCBVERS_STAT) || (success == FALSE))
		return;
	inf[rtype].unsetinfo++;
	return;
}

void
rpcbs_getaddr(rpcvers_t rtype, rpcprog_t prog, rpcvers_t vers, char *netid,
	      char *uaddr)
{
	rpcbs_addrlist *al;
	struct netconfig *nconf;

	if (rtype >= RPCBVERS_STAT)
		return;
	for (al = inf[rtype].addrinfo; al; al = al->next) {

		if(al->netid == NULL)
			return;
		if ((al->prog == prog) && (al->vers == vers) &&
		    (strcmp(al->netid, netid) == 0)) {
			if ((uaddr == NULL) || (uaddr[0] == 0))
				al->failure++;
			else
				al->success++;
			return;
		}
	}
	nconf = rpcbind_get_conf(netid);
	if (nconf == NULL) {
		return;
	}
	al = (rpcbs_addrlist *) malloc(sizeof (rpcbs_addrlist));
	if (al == NULL) {
		return;
	}
	al->prog = prog;
	al->vers = vers;
	al->netid = nconf->nc_netid;
	if ((uaddr == NULL) || (uaddr[0] == 0)) {
		al->failure = 1;
		al->success = 0;
	} else {
		al->failure = 0;
		al->success = 1;
	}
	al->next = inf[rtype].addrinfo;
	inf[rtype].addrinfo = al;
}

void
rpcbs_rmtcall(rpcvers_t rtype, rpcproc_t rpcbproc, rpcprog_t prog,
	      rpcvers_t vers, rpcproc_t proc, char *netid, rpcblist_ptr rbl)
{
	rpcbs_rmtcalllist *rl;
	struct netconfig *nconf;

	if (rtype > RPCBVERS_STAT)
		return;
	for (rl = inf[rtype].rmtinfo; rl; rl = rl->next) {

		if(rl->netid == NULL)
			return;

		if ((rl->prog == prog) && (rl->vers == vers) &&
		    (rl->proc == proc) &&
		    (strcmp(rl->netid, netid) == 0)) {
			if ((rbl == NULL) ||
			    (rbl->rpcb_map.r_vers != vers))
				rl->failure++;
			else
				rl->success++;
			if (rpcbproc == RPCBPROC_INDIRECT)
				rl->indirect++;
			return;
		}
	}
	nconf = rpcbind_get_conf(netid);
	if (nconf == NULL) {
		return;
	}
	rl = (rpcbs_rmtcalllist *) malloc(sizeof (rpcbs_rmtcalllist));
	if (rl == NULL) {
		return;
	}
	rl->prog = prog;
	rl->vers = vers;
	rl->proc = proc;
	rl->netid = nconf->nc_netid;
	if ((rbl == NULL) ||
		    (rbl->rpcb_map.r_vers != vers)) {
		rl->failure = 1;
		rl->success = 0;
	} else {
		rl->failure = 0;
		rl->success = 1;
	}
	rl->indirect = 1;
	rl->next = inf[rtype].rmtinfo;
	inf[rtype].rmtinfo = rl;
	return;
}

void *
rpcbproc_getstat(void *arg __unused, struct svc_req *req __unused,
    SVCXPRT *xprt __unused, rpcvers_t versnum __unused)
{
	return (void *)&inf;
}