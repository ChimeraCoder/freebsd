
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

#if HAVE_CONFIG_H
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

#include <mad.h>
#include <infiniband/common.h>

#undef DEBUG
#define DEBUG 	if (ibdebug)	IBWARN

uint8_t *
sa_rpc_call(const void *ibmad_port, void *rcvbuf, ib_portid_t *portid,
	    ib_sa_call_t *sa, unsigned timeout)
{
	ib_rpc_t rpc = {0};
	uint8_t *p;

	DEBUG("attr 0x%x mod 0x%x route %s", sa->attrid, sa->mod,
	      portid2str(portid));

	if (portid->lid <= 0) {
		IBWARN("only lid routes are supported");
		return 0;
	}

	rpc.mgtclass = IB_SA_CLASS;
	rpc.method = sa->method;
	rpc.attr.id = sa->attrid;
	rpc.attr.mod = sa->mod;
	rpc.mask = sa->mask;
	rpc.timeout = timeout;
	rpc.datasz = IB_SA_DATA_SIZE;
	rpc.dataoffs = IB_SA_DATA_OFFS;
	rpc.trid = sa->trid;

	portid->qp = 1;
	if (!portid->qkey)
		portid->qkey = IB_DEFAULT_QP1_QKEY;

	p = mad_rpc_rmpp(ibmad_port, &rpc, portid, 0/*&sa->rmpp*/, rcvbuf);	/* TODO: RMPP */

	sa->recsz = rpc.recsz;

	return p;
}

/* PathRecord */
#define IB_PR_COMPMASK_DGID				(1ull<<2)
#define IB_PR_COMPMASK_SGID				(1ull<<3)
#define IB_PR_COMPMASK_DLID				(1ull<<4)
#define	IB_PR_COMPMASK_SLID				(1ull<<5)
#define	IB_PR_COMPMASK_RAWTRAFIC			(1ull<<6)
#define	IB_PR_COMPMASK_RESV0				(1ull<<7)
#define	IB_PR_COMPMASK_FLOWLABEL			(1ull<<8)
#define	IB_PR_COMPMASK_HOPLIMIT				(1ull<<9)
#define	IB_PR_COMPMASK_TCLASS				(1ull<<10)
#define	IB_PR_COMPMASK_REVERSIBLE			(1ull<<11)
#define	IB_PR_COMPMASK_NUMBPATH				(1ull<<12)
#define	IB_PR_COMPMASK_PKEY				(1ull<<13)
#define	IB_PR_COMPMASK_RESV1				(1ull<<14)
#define	IB_PR_COMPMASK_SL				(1ull<<15)
#define	IB_PR_COMPMASK_MTUSELEC				(1ull<<16)
#define	IB_PR_COMPMASK_MTU				(1ull<<17)
#define	IB_PR_COMPMASK_RATESELEC			(1ull<<18)
#define	IB_PR_COMPMASK_RATE				(1ull<<19)
#define	IB_PR_COMPMASK_PKTLIFETIMESELEC			(1ull<<20)
#define	IB_PR_COMPMASK_PKTLIFETIME			(1ull<<21)
#define	IB_PR_COMPMASK_PREFERENCE			(1ull<<22)

#define IB_PR_DEF_MASK (IB_PR_COMPMASK_DGID |\
			IB_PR_COMPMASK_SGID |\
			IB_PR_COMPMASK_NUMBPATH)

int
ib_path_query_via(const void *srcport, ibmad_gid_t srcgid, ibmad_gid_t destgid, ib_portid_t *sm_id, void *buf)
{
	int npath;
	ib_sa_call_t sa = {0};
	uint8_t *p;
	int dlid;

	npath = 1;			/* only MAD_METHOD_GET is supported */
	memset(&sa, 0, sizeof sa);
	sa.method = IB_MAD_METHOD_GET;
	sa.attrid = IB_SA_ATTR_PATHRECORD;
	sa.mask = IB_PR_DEF_MASK;
	sa.trid = mad_trid();

	memset(buf, 0, IB_SA_PR_RECSZ);

	mad_encode_field(buf, IB_SA_PR_NPATH_F, &npath);
	mad_encode_field(buf, IB_SA_PR_DGID_F, destgid);
	mad_encode_field(buf, IB_SA_PR_SGID_F, srcgid);

	if (srcport) {
		p = sa_rpc_call (srcport, buf, sm_id, &sa, 0);
	} else {
		p = safe_sa_call(buf, sm_id, &sa, 0);
	}
	if (!p) {
		IBWARN("sa call path_query failed");
		return -1;
	}

	mad_decode_field(p, IB_SA_PR_DLID_F, &dlid);
	return dlid;
}
int
ib_path_query(ibmad_gid_t srcgid, ibmad_gid_t destgid, ib_portid_t *sm_id, void *buf)
{
	return ib_path_query_via (NULL, srcgid, destgid, sm_id, buf);
}