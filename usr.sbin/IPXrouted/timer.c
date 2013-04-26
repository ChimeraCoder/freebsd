
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

#ifndef lint
static const char sccsid[] = "@(#)timer.c	8.1 (Berkeley) 6/5/93";
#endif /* not lint */

/*
 * Routing Table Management Daemon
 */
#include "defs.h"
#include <unistd.h>
#include <stdlib.h>

int	timeval = -TIMER_RATE;

/*
 * Timer routine.  Performs routing information supply
 * duties and manages timers on routing and SAP table entries.
 */
void
timer()
{
	register struct rthash *rh;
	register struct rt_entry *rt;
	register struct sap_hash *sh;
	register struct sap_entry *sap;
	struct sap_hash *sap_base = sap_head;
	int timetobroadcast, ripbroadcast, sapbroadcast;

	timeval += TIMER_RATE;
	if (lookforinterfaces && (timeval % CHECK_INTERVAL) == 0)
		ifinit();
	timetobroadcast = supplier && (timeval % SUPPLY_INTERVAL) == 0;
	ripbroadcast = supplier && timetobroadcast && 
			(timeval % RIP_INTERVAL) == 0;
	sapbroadcast = timetobroadcast && dosap && !ripbroadcast;

	for (rh = nethash; rh < &nethash[ROUTEHASHSIZ]; rh++) {
		rt = rh->rt_forw;
		for (; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
			if (rt->rt_clone) {
				struct rt_entry *trt, *prt;
				/*
				 * If a clone expire free it and mark the
				 * main route RTS_CHANGED.
				 */
				prt = rt;
				trt = rt->rt_clone;
				while (trt) {
					trt->rt_timer += TIMER_RATE;
					if (trt->rt_timer >= EXPIRE_TIME) {
						prt->rt_clone = trt->rt_clone;
						free((char *)trt);
						trt = prt->rt_clone;
						rt->rt_state |= RTS_CHANGED;
					} else {
						prt = trt;
						trt = prt->rt_clone;
					}
				}
			}
			/*
			 * We don't advance time on a routing entry for
			 * a passive gateway or that for our only interface. 
			 * The latter is excused because we don't act as
			 * a routing information supplier and hence would
			 * time it out.  This is fair as if it's down
			 * we're cut off from the world anyway and it's
			 * not likely we'll grow any new hardware in
			 * the mean time.
			 */
			if (!(rt->rt_state & RTS_PASSIVE) &&
			    !(rt->rt_state & RTS_INTERFACE))
				rt->rt_timer += TIMER_RATE;
			if (rt->rt_timer >= EXPIRE_TIME) {
				rt->rt_metric = HOPCNT_INFINITY;
				rt->rt_state |= RTS_CHANGED;
			}
			if (rt->rt_timer >= GARBAGE_TIME) {
				rt = rt->rt_back;
				/* Perhaps we should send a REQUEST for this route? */
				rtdelete(rt->rt_forw);
				continue;
			}
			if (rt->rt_state & RTS_CHANGED) {
				rt->rt_state &= ~RTS_CHANGED;
				/* don't send extraneous packets */
				if (!supplier || ripbroadcast)
					continue;
				if ((rt->rt_metric + 1) == HOPCNT_INFINITY)
					continue;
				msg->rip_cmd = htons(RIPCMD_RESPONSE);
				msg->rip_nets[0].rip_dst =
					(satoipx_addr(rt->rt_dst)).x_net;
				msg->rip_nets[0].rip_metric =
				   	htons(min(rt->rt_metric+1, HOPCNT_INFINITY));
				msg->rip_nets[0].rip_ticks = 
					htons(rt->rt_ticks + 1);
				toall(sndmsg, rt, 0);
			}
		}
	}
	if (ripbroadcast)
		toall(supply, NULL, 0);

	/* 
	 * Now do the SAP stuff.
	 */
	for (sh = sap_base; sh < &sap_base[SAPHASHSIZ]; sh++) {
		sap = sh->forw;
		for (; sap != (struct sap_entry *)sh; sap = sap->forw) {
			if (sap->clone) {
				struct sap_entry *tsap, *psap;
				/*
				 * If a clone expire free it and mark the
				 * main sap entry RTS_CHANGED.
				 */
				psap = sap;
				tsap = sap->clone;
				while (tsap) {
					tsap->timer += TIMER_RATE;
					if (tsap->timer >= EXPIRE_TIME) {
						psap->clone = tsap->clone;
						free((char *)tsap);
						tsap = psap->clone;
						sap->state |= RTS_CHANGED;
					} else {
						psap = tsap;
						tsap = psap->clone;
					}
				}
			}
			sap->timer += TIMER_RATE;
			if (sap->timer >= EXPIRE_TIME) {
				sap->sap.hops = htons(HOPCNT_INFINITY);
				sap->state |= RTS_CHANGED;
			}
			if (sap->timer >= GARBAGE_TIME) {
				sap = sap->back;
				/* Perhaps we should send a REQUEST for this route? */
				sap_delete(sap->forw);
				continue;
			}
			/*
			 * XXX sap_sndmsg on RTS_CHANGED
			 */
			if (sap->state & RTS_CHANGED) {
				sap->state &= ~RTS_CHANGED;
#ifdef notyet
				/* don't send extraneous packets */
				if (!supplier || sapbroadcast)
					continue;
				if ((ntohs(sap->sap.hops) + 1) == HOPCNT_INFINITY)
					continue;
				sap_msg->sap_cmd = htons(SAP_RESP);
				sap_msg->sap[0] = sap->sap;
				sap_msg->sap[0].hops =
				    htons(min(sap->sap.hops+1, HOPCNT_INFINITY));
				toall(sapsndmsg, rt, 0);
#endif
			}
		}
	}
	if (sapbroadcast)
		sap_supply_toall(0);
	if (ftrace && sapbroadcast)
		dumpsaptable(ftrace, sap_head);
}

/*
 * On hangup, let everyone know we're going away.
 */
void
hup()
{
	register struct rthash *rh;
	register struct rt_entry *rt;
	register struct sap_hash *sh;
	register struct sap_entry *sap;

	if (supplier) {
		for (rh = nethash; rh < &nethash[ROUTEHASHSIZ]; rh++) {
			rt = rh->rt_forw;
			for (; rt != (struct rt_entry *)rh; rt = rt->rt_forw)
				rt->rt_metric = HOPCNT_INFINITY;
		}
		toall(supply, NULL, 0);

		/*
		 * Now for SAP.
		 */
		for (sh = sap_head; sh < &sap_head[SAPHASHSIZ]; sh++) {
			sap = sh->forw;
			for (; sap != (struct sap_entry *)sh; sap = sap->forw)
				sap->sap.hops = htons(HOPCNT_INFINITY);
		}
		if (dosap)
			sap_supply_toall(0);
	}
	exit(1);
}