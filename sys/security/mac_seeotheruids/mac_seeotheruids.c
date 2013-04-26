
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
 * Developed by the TrustedBSD Project.
 *
 * Prevent processes owned by a particular uid from seeing various transient
 * kernel objects associated with other uids.
 */

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/priv.h>
#include <sys/proc.h>
#include <sys/systm.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/sysctl.h>

#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_pcb.h>

#include <security/mac/mac_policy.h>

SYSCTL_DECL(_security_mac);

static SYSCTL_NODE(_security_mac, OID_AUTO, seeotheruids, CTLFLAG_RW, 0,
    "TrustedBSD mac_seeotheruids policy controls");

static int	seeotheruids_enabled = 1;
SYSCTL_INT(_security_mac_seeotheruids, OID_AUTO, enabled, CTLFLAG_RW,
    &seeotheruids_enabled, 0, "Enforce seeotheruids policy");

/*
 * Exception: allow credentials to be aware of other credentials with the
 * same primary gid.
 */
static int	primarygroup_enabled = 0;
SYSCTL_INT(_security_mac_seeotheruids, OID_AUTO, primarygroup_enabled,
    CTLFLAG_RW, &primarygroup_enabled, 0, "Make an exception for credentials "
    "with the same real primary group id");

/*
 * Exception: allow the root user to be aware of other credentials by virtue
 * of privilege.
 */
static int	suser_privileged = 1;
SYSCTL_INT(_security_mac_seeotheruids, OID_AUTO, suser_privileged,
    CTLFLAG_RW, &suser_privileged, 0, "Make an exception for superuser");

/*
 * Exception: allow processes with a specific gid to be exempt from the
 * policy.  One sysctl enables this functionality; the other sets the
 * exempt gid.
 */
static int	specificgid_enabled = 0;
SYSCTL_INT(_security_mac_seeotheruids, OID_AUTO, specificgid_enabled,
    CTLFLAG_RW, &specificgid_enabled, 0, "Make an exception for credentials "
    "with a specific gid as their real primary group id or group set");

static gid_t	specificgid = 0;
SYSCTL_UINT(_security_mac_seeotheruids, OID_AUTO, specificgid, CTLFLAG_RW,
    &specificgid, 0, "Specific gid to be exempt from seeotheruids policy");

static int
seeotheruids_check(struct ucred *cr1, struct ucred *cr2)
{

	if (!seeotheruids_enabled)
		return (0);

	if (primarygroup_enabled) {
		if (cr1->cr_rgid == cr2->cr_rgid)
			return (0);
	}

	if (specificgid_enabled) {
		if (cr1->cr_rgid == specificgid ||
		    groupmember(specificgid, cr1))
			return (0);
	}

	if (cr1->cr_ruid == cr2->cr_ruid)
		return (0);

	if (suser_privileged) {
		if (priv_check_cred(cr1, PRIV_SEEOTHERUIDS, 0) == 0)
			return (0);
	}

	return (ESRCH);
}

static int
seeotheruids_proc_check_debug(struct ucred *cred, struct proc *p)
{

	return (seeotheruids_check(cred, p->p_ucred));
}

static int
seeotheruids_proc_check_sched(struct ucred *cred, struct proc *p)
{

	return (seeotheruids_check(cred, p->p_ucred));
}

static int
seeotheruids_proc_check_signal(struct ucred *cred, struct proc *p,
    int signum)
{

	return (seeotheruids_check(cred, p->p_ucred));
}

static int
seeotheruids_cred_check_visible(struct ucred *cr1, struct ucred *cr2)
{

	return (seeotheruids_check(cr1, cr2));
}

static int
seeotheruids_inpcb_check_visible(struct ucred *cred, struct inpcb *inp,
    struct label *inplabel)
{

	return (seeotheruids_check(cred, inp->inp_cred));
}

static int
seeotheruids_socket_check_visible(struct ucred *cred, struct socket *so,
    struct label *solabel)
{

	return (seeotheruids_check(cred, so->so_cred));
}

static struct mac_policy_ops seeotheruids_ops =
{
	.mpo_proc_check_debug = seeotheruids_proc_check_debug,
	.mpo_proc_check_sched = seeotheruids_proc_check_sched,
	.mpo_proc_check_signal = seeotheruids_proc_check_signal,
	.mpo_cred_check_visible = seeotheruids_cred_check_visible,
	.mpo_inpcb_check_visible = seeotheruids_inpcb_check_visible,
	.mpo_socket_check_visible = seeotheruids_socket_check_visible,
};

MAC_POLICY_SET(&seeotheruids_ops, mac_seeotheruids,
    "TrustedBSD MAC/seeotheruids", MPC_LOADTIME_FLAG_UNLOADOK, NULL);