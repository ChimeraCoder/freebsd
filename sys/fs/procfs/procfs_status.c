
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
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/exec.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/jail.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/sx.h>
#include <sys/proc.h>
#include <sys/resourcevar.h>
#include <sys/sbuf.h>
#include <sys/sysent.h>
#include <sys/tty.h>

#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/vm_param.h>

#include <fs/pseudofs/pseudofs.h>
#include <fs/procfs/procfs.h>

int
procfs_doprocstatus(PFS_FILL_ARGS)
{
	struct session *sess;
	struct thread *tdfirst;
	struct tty *tp;
	struct ucred *cr;
	const char *wmesg;
	char *pc;
	char *sep;
	int pid, ppid, pgid, sid;
	int i;

	pid = p->p_pid;
	PROC_LOCK(p);
	ppid = p->p_pptr ? p->p_pptr->p_pid : 0;
	pgid = p->p_pgrp->pg_id;
	sess = p->p_pgrp->pg_session;
	SESS_LOCK(sess);
	sid = sess->s_leader ? sess->s_leader->p_pid : 0;

/* comm pid ppid pgid sid tty ctty,sldr start ut st wmsg
				euid ruid rgid,egid,groups[1 .. ngroups]
*/

	pc = p->p_comm;
	do {
		if (*pc < 33 || *pc > 126 || *pc == '\\')
			sbuf_printf(sb, "\\%03o", *pc);
		else
			sbuf_putc(sb, *pc);
	} while (*++pc);
	sbuf_printf(sb, " %d %d %d %d ", pid, ppid, pgid, sid);
	if ((p->p_flag & P_CONTROLT) && (tp = sess->s_ttyp))
		sbuf_printf(sb, "%s ", devtoname(tp->t_dev));
	else
		sbuf_printf(sb, "- ");

	sep = "";
	if (sess->s_ttyvp) {
		sbuf_printf(sb, "%sctty", sep);
		sep = ",";
	}
	if (SESS_LEADER(p)) {
		sbuf_printf(sb, "%ssldr", sep);
		sep = ",";
	}
	SESS_UNLOCK(sess);
	if (*sep != ',') {
		sbuf_printf(sb, "noflags");
	}

	tdfirst = FIRST_THREAD_IN_PROC(p);
	thread_lock(tdfirst);
	if (tdfirst->td_wchan != NULL) {
		KASSERT(tdfirst->td_wmesg != NULL,
		    ("wchan %p has no wmesg", tdfirst->td_wchan));
		wmesg = tdfirst->td_wmesg;
	} else
		wmesg = "nochan";
	thread_unlock(tdfirst);

	if (p->p_flag & P_INMEM) {
		struct timeval start, ut, st;

		PROC_SLOCK(p);
		calcru(p, &ut, &st);
		PROC_SUNLOCK(p);
		start = p->p_stats->p_start;
		timevaladd(&start, &boottime);
		sbuf_printf(sb, " %jd,%ld %jd,%ld %jd,%ld",
		    (intmax_t)start.tv_sec, start.tv_usec,
		    (intmax_t)ut.tv_sec, ut.tv_usec,
		    (intmax_t)st.tv_sec, st.tv_usec);
	} else
		sbuf_printf(sb, " -1,-1 -1,-1 -1,-1");

	sbuf_printf(sb, " %s", wmesg);

	cr = p->p_ucred;

	sbuf_printf(sb, " %lu %lu %lu",
		(u_long)cr->cr_uid,
		(u_long)cr->cr_ruid,
		(u_long)cr->cr_rgid);

	/* egid (cr->cr_svgid) is equal to cr_ngroups[0]
	   see also getegid(2) in /sys/kern/kern_prot.c */

	for (i = 0; i < cr->cr_ngroups; i++) {
		sbuf_printf(sb, ",%lu", (u_long)cr->cr_groups[i]);
	}

	if (jailed(cr)) {
		mtx_lock(&cr->cr_prison->pr_mtx);
		sbuf_printf(sb, " %s",
		    prison_name(td->td_ucred->cr_prison, cr->cr_prison));
		mtx_unlock(&cr->cr_prison->pr_mtx);
	} else {
		sbuf_printf(sb, " -");
	}
	PROC_UNLOCK(p);
	sbuf_printf(sb, "\n");

	return (0);
}

int
procfs_doproccmdline(PFS_FILL_ARGS)
{

	/*
	 * If we are using the ps/cmdline caching, use that.  Otherwise
	 * read argv from the process space.
	 * Note that if the argv is no longer available, we deliberately
	 * don't fall back on p->p_comm or return an error: the authentic
	 * Linux behaviour is to return zero-length in this case.
	 */

	PROC_LOCK(p);
	if (p->p_args && p_cansee(td, p) == 0) {
		sbuf_bcpy(sb, p->p_args->ar_args, p->p_args->ar_length);
		PROC_UNLOCK(p);
		return (0);
	}

	if ((p->p_flag & P_SYSTEM) != 0) {
		PROC_UNLOCK(p);
		return (0);
	}

	PROC_UNLOCK(p);

	return (proc_getargv(td, p, sb));
}