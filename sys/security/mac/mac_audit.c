
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

#include "opt_kdtrace.h"

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/queue.h>
#include <sys/sdt.h>
#include <sys/vnode.h>

#include <security/audit/audit.h>

#include <security/mac/mac_framework.h>
#include <security/mac/mac_internal.h>
#include <security/mac/mac_policy.h>

MAC_CHECK_PROBE_DEFINE2(cred_check_setaudit, "struct ucred *",
    "struct auditinfo *");

int
mac_cred_check_setaudit(struct ucred *cred, struct auditinfo *ai)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(cred_check_setaudit, cred, ai);
	MAC_CHECK_PROBE2(cred_check_setaudit, error, cred, ai);

	return (error);
}

MAC_CHECK_PROBE_DEFINE2(cred_check_setaudit_addr, "struct ucred *",
    "struct auditinfo_addr *");

int
mac_cred_check_setaudit_addr(struct ucred *cred, struct auditinfo_addr *aia)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(cred_check_setaudit_addr, cred, aia);
	MAC_CHECK_PROBE2(cred_check_setaudit_addr, error, cred, aia);

	return (error);
}

MAC_CHECK_PROBE_DEFINE2(cred_check_setauid, "struct ucred *", "uid_t");

int
mac_cred_check_setauid(struct ucred *cred, uid_t auid)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(cred_check_setauid, cred, auid);
	MAC_CHECK_PROBE2(cred_check_setauid, error, cred, auid);

	return (error);
}

MAC_CHECK_PROBE_DEFINE3(system_check_audit, "struct ucred *", "void *",
    "int");

int
mac_system_check_audit(struct ucred *cred, void *record, int length)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(system_check_audit, cred, record, length);
	MAC_CHECK_PROBE3(system_check_audit, error, cred, record, length);

	return (error);
}

MAC_CHECK_PROBE_DEFINE2(system_check_auditctl, "struct ucred *",
    "struct vnode *");

int
mac_system_check_auditctl(struct ucred *cred, struct vnode *vp)
{
	int error;
	struct label *vl;

	ASSERT_VOP_LOCKED(vp, "mac_system_check_auditctl");

	vl = (vp != NULL) ? vp->v_label : NULL;
	MAC_POLICY_CHECK(system_check_auditctl, cred, vp, vl);
	MAC_CHECK_PROBE2(system_check_auditctl, error, cred, vp);

	return (error);
}

MAC_CHECK_PROBE_DEFINE2(system_check_auditon, "struct ucred *", "int");

int
mac_system_check_auditon(struct ucred *cred, int cmd)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(system_check_auditon, cred, cmd);
	MAC_CHECK_PROBE2(system_check_auditon, error, cred, cmd);

	return (error);
}