
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
 * $FreeBSD$
 */

#ifdef _KERNEL
#include <sys/malloc.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/kernel.h>
#include <sys/mbuf.h>
#include <sys/module.h>
#include <net/if.h>	/* IFNAMSIZ */
#include <netinet/in.h>
#include <netinet/ip_var.h>		/* ipfw_rule_ref */
#include <netinet/ip_fw.h>	/* flow_id */
#include <netinet/ip_dummynet.h>
#include <netpfil/ipfw/dn_heap.h>
#include <netpfil/ipfw/ip_dn_private.h>
#include <netpfil/ipfw/dn_sched.h>
#else
#include <dn_test.h>
#endif

/*
 * This file implements a FIFO scheduler for a single queue.
 * The queue is allocated as part of the scheduler instance,
 * and there is a single flowset is in the template which stores
 * queue size and policy.
 * Enqueue and dequeue use the default library functions.
 */
static int 
fifo_enqueue(struct dn_sch_inst *si, struct dn_queue *q, struct mbuf *m)
{
	/* XXX if called with q != NULL and m=NULL, this is a
	 * re-enqueue from an existing scheduler, which we should
	 * handle.
	 */
	return dn_enqueue((struct dn_queue *)(si+1), m, 0);
}

static struct mbuf *
fifo_dequeue(struct dn_sch_inst *si)
{
	return dn_dequeue((struct dn_queue *)(si + 1));
}

static int
fifo_new_sched(struct dn_sch_inst *si)
{
	/* This scheduler instance contains the queue */
	struct dn_queue *q = (struct dn_queue *)(si + 1);

        set_oid(&q->ni.oid, DN_QUEUE, sizeof(*q));
	q->_si = si;
	q->fs = si->sched->fs;
	return 0;
}

static int
fifo_free_sched(struct dn_sch_inst *si)
{
	struct dn_queue *q = (struct dn_queue *)(si + 1);
	dn_free_pkts(q->mq.head);
	bzero(q, sizeof(*q));
	return 0;
}

/*
 * FIFO scheduler descriptor
 * contains the type of the scheduler, the name, the size of extra
 * data structures, and function pointers.
 */
static struct dn_alg fifo_desc = {
	_SI( .type = )  DN_SCHED_FIFO,
	_SI( .name = )  "FIFO",
	_SI( .flags = ) 0,

	_SI( .schk_datalen = ) 0,
	_SI( .si_datalen = )  sizeof(struct dn_queue),
	_SI( .q_datalen = )  0,

	_SI( .enqueue = )  fifo_enqueue,
	_SI( .dequeue = )  fifo_dequeue,
	_SI( .config = )  NULL,
	_SI( .destroy = )  NULL,
	_SI( .new_sched = )  fifo_new_sched,
	_SI( .free_sched = )  fifo_free_sched,
	_SI( .new_fsk = )  NULL,
	_SI( .free_fsk = )  NULL,
	_SI( .new_queue = )  NULL,
	_SI( .free_queue = )  NULL,
};

DECLARE_DNSCHED_MODULE(dn_fifo, &fifo_desc);