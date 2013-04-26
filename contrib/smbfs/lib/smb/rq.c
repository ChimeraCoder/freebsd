
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
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sysexits.h>

#include <sys/mchain.h>

#include <netsmb/smb_lib.h>
#include <netsmb/smb_conn.h>
#include <netsmb/smb_rap.h>


int
smb_rq_init(struct smb_ctx *ctx, u_char cmd, size_t rpbufsz, struct smb_rq **rqpp)
{
	struct smb_rq *rqp;

	rqp = malloc(sizeof(*rqp));
	if (rqp == NULL)
		return ENOMEM;
	bzero(rqp, sizeof(*rqp));
	rqp->rq_cmd = cmd;
	rqp->rq_ctx = ctx;
	mb_init(&rqp->rq_rq, M_MINSIZE);
	mb_init(&rqp->rq_rp, rpbufsz);
	*rqpp = rqp;
	return 0;
}

void
smb_rq_done(struct smb_rq *rqp)
{
	mb_done(&rqp->rq_rp);
	mb_done(&rqp->rq_rq);
	free(rqp);
}

void
smb_rq_wend(struct smb_rq *rqp)
{
	if (rqp->rq_rq.mb_count & 1)
		smb_error("smbrq_wend: odd word count\n", 0);
	rqp->rq_wcount = rqp->rq_rq.mb_count / 2;
	rqp->rq_rq.mb_count = 0;
}

int
smb_rq_dmem(struct mbdata *mbp, const char *src, size_t size)
{
	struct mbuf *m;
	char * dst;
	int cplen, error;

	if (size == 0)
		return 0;
	m = mbp->mb_cur;
	if ((error = m_getm(m, size, &m)) != 0)
		return error;
	while (size > 0) {
		cplen = M_TRAILINGSPACE(m);
		if (cplen == 0) {
			m = m->m_next;
			continue;
		}
		if (cplen > (int)size)
			cplen = size;
		dst = mtod(m, char *) + m->m_len;
		nls_mem_toext(dst, src, cplen);
		size -= cplen;
		src += cplen;
		m->m_len += cplen;
		mbp->mb_count += cplen;
	}
	mbp->mb_pos = mtod(m, char *) + m->m_len;
	mbp->mb_cur = m;
	return 0;
}

int
smb_rq_dstring(struct mbdata *mbp, const char *s)
{
	return smb_rq_dmem(mbp, s, strlen(s) + 1);
}

int
smb_rq_simple(struct smb_rq *rqp)
{
	struct smbioc_rq krq;
	struct mbdata *mbp;
	char *data;

	mbp = smb_rq_getrequest(rqp);
	m_lineup(mbp->mb_top, &mbp->mb_top);
	data = mtod(mbp->mb_top, char*);
	bzero(&krq, sizeof(krq));
	krq.ioc_cmd = rqp->rq_cmd;
	krq.ioc_twc = rqp->rq_wcount;
	krq.ioc_twords = data;
	krq.ioc_tbc = mbp->mb_count;
	krq.ioc_tbytes = data + rqp->rq_wcount * 2;
	mbp = smb_rq_getreply(rqp);
	krq.ioc_rpbufsz = mbp->mb_top->m_maxlen;
	krq.ioc_rpbuf = mtod(mbp->mb_top, char *);
	if (ioctl(rqp->rq_ctx->ct_fd, SMBIOC_REQUEST, &krq) == -1)
		return errno;
	mbp->mb_top->m_len = krq.ioc_rwc * 2 + krq.ioc_rbc;
	rqp->rq_wcount = krq.ioc_rwc;
	rqp->rq_bcount = krq.ioc_rbc;
	return 0;
}

int
smb_t2_request(struct smb_ctx *ctx, int setup, int setupcount,
	const char *name,
	int tparamcnt, void *tparam,
	int tdatacnt, void *tdata,
	int *rparamcnt, void *rparam,
	int *rdatacnt, void *rdata)
{
	struct smbioc_t2rq krq;

	bzero(&krq, sizeof(krq));
	krq.ioc_setup[0] = setup;
	krq.ioc_setupcnt = setupcount;
	krq.ioc_name = (char *)name;
	krq.ioc_tparamcnt = tparamcnt;
	krq.ioc_tparam = tparam;
	krq.ioc_tdatacnt = tdatacnt;
	krq.ioc_tdata = tdata;
	krq.ioc_rparamcnt = *rparamcnt;
	krq.ioc_rparam = rparam;
	krq.ioc_rdatacnt = *rdatacnt;
	krq.ioc_rdata = rdata;
	if (ioctl(ctx->ct_fd, SMBIOC_T2RQ, &krq) == -1)
		return errno;
	*rparamcnt = krq.ioc_rparamcnt;
	*rdatacnt = krq.ioc_rdatacnt;
	return 0;
}