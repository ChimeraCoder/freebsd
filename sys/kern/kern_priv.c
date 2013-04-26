
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

#include "opt_kdtrace.h"

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/jail.h>
#include <sys/kernel.h>
#include <sys/priv.h>
#include <sys/proc.h>
#include <sys/sdt.h>
#include <sys/sysctl.h>
#include <sys/systm.h>

#include <security/mac/mac_framework.h>

/*
 * `suser_enabled' (which can be set by the security.bsd.suser_enabled
 * sysctl) determines whether the system 'super-user' policy is in effect.  If
 * it is nonzero, an effective uid of 0 connotes special privilege,
 * overriding many mandatory and discretionary protections.  If it is zero,
 * uid 0 is offered no special privilege in the kernel security policy.
 * Setting it to zero may seriously impact the functionality of many existing
 * userland programs, and should not be done without careful consideration of
 * the consequences. 
 */
static int	suser_enabled = 1;
SYSCTL_INT(_security_bsd, OID_AUTO, suser_enabled, CTLFLAG_RW,
    &suser_enabled, 0, "processes with uid 0 have privilege");
TUNABLE_INT("security.bsd.suser_enabled", &suser_enabled);

static int	unprivileged_mlock = 1;
SYSCTL_INT(_security_bsd, OID_AUTO, unprivileged_mlock, CTLFLAG_RW|CTLFLAG_TUN,
    &unprivileged_mlock, 0, "Allow non-root users to call mlock(2)");
TUNABLE_INT("security.bsd.unprivileged_mlock", &unprivileged_mlock);

SDT_PROVIDER_DEFINE(priv);
SDT_PROBE_DEFINE1(priv, kernel, priv_check, priv_ok, priv-ok, "int");
SDT_PROBE_DEFINE1(priv, kernel, priv_check, priv_err, priv-err, "int");

/*
 * Check a credential for privilege.  Lots of good reasons to deny privilege;
 * only a few to grant it.
 */
int
priv_check_cred(struct ucred *cred, int priv, int flags)
{
	int error;

	KASSERT(PRIV_VALID(priv), ("priv_check_cred: invalid privilege %d",
	    priv));

	/*
	 * We first evaluate policies that may deny the granting of
	 * privilege unilaterally.
	 */
#ifdef MAC
	error = mac_priv_check(cred, priv);
	if (error)
		goto out;
#endif

	/*
	 * Jail policy will restrict certain privileges that may otherwise be
	 * be granted.
	 */
	error = prison_priv_check(cred, priv);
	if (error)
		goto out;

	if (unprivileged_mlock) {
		/*
		 * Allow unprivileged users to call mlock(2)/munlock(2) and
		 * mlockall(2)/munlockall(2).
		 */
		switch (priv) {
			case PRIV_VM_MLOCK:
			case PRIV_VM_MUNLOCK:
				error = 0;
				goto out;
		}
	}

	/*
	 * Having determined if privilege is restricted by various policies,
	 * now determine if privilege is granted.  At this point, any policy
	 * may grant privilege.  For now, we allow short-circuit boolean
	 * evaluation, so may not call all policies.  Perhaps we should.
	 *
	 * Superuser policy grants privilege based on the effective (or in
	 * the case of specific privileges, real) uid being 0.  We allow the
	 * superuser policy to be globally disabled, although this is
	 * currenty of limited utility.
	 */
	if (suser_enabled) {
		switch (priv) {
		case PRIV_MAXFILES:
		case PRIV_MAXPROC:
		case PRIV_PROC_LIMIT:
			if (cred->cr_ruid == 0) {
				error = 0;
				goto out;
			}
			break;

		default:
			if (cred->cr_uid == 0) {
				error = 0;
				goto out;
			}
			break;
		}
	}

	/*
	 * Now check with MAC, if enabled, to see if a policy module grants
	 * privilege.
	 */
#ifdef MAC
	if (mac_priv_grant(cred, priv) == 0) {
		error = 0;
		goto out;
	}
#endif

	/*
	 * The default is deny, so if no policies have granted it, reject
	 * with a privilege error here.
	 */
	error = EPERM;
out:
	if (error) {
		SDT_PROBE(priv, kernel, priv_check, priv_err, priv, 0, 0, 0,
		    0);
	} else {
		SDT_PROBE(priv, kernel, priv_check, priv_ok, priv, 0, 0, 0,
		    0);
	}
	return (error);
}

int
priv_check(struct thread *td, int priv)
{

	KASSERT(td == curthread, ("priv_check: td != curthread"));

	return (priv_check_cred(td->td_ucred, priv, 0));
}