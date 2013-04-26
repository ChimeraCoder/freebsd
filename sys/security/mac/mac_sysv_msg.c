
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
#include "opt_mac.h"

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/sbuf.h>
#include <sys/sdt.h>
#include <sys/systm.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/file.h>
#include <sys/namei.h>
#include <sys/sysctl.h>
#include <sys/msg.h>

#include <security/mac/mac_framework.h>
#include <security/mac/mac_internal.h>
#include <security/mac/mac_policy.h>

static struct label *
mac_sysv_msgmsg_label_alloc(void)
{
	struct label *label;

	label = mac_labelzone_alloc(M_WAITOK);
	MAC_POLICY_PERFORM(sysvmsg_init_label, label);
	return (label);
}

void
mac_sysvmsg_init(struct msg *msgptr)
{

	if (mac_labeled & MPC_OBJECT_SYSVMSG)
		msgptr->label = mac_sysv_msgmsg_label_alloc();
	else
		msgptr->label = NULL;
}

static struct label *
mac_sysv_msgqueue_label_alloc(void)
{
	struct label *label;

	label = mac_labelzone_alloc(M_WAITOK);
	MAC_POLICY_PERFORM(sysvmsq_init_label, label);
	return (label);
}

void
mac_sysvmsq_init(struct msqid_kernel *msqkptr)
{

	if (mac_labeled & MPC_OBJECT_SYSVMSQ)
		msqkptr->label = mac_sysv_msgqueue_label_alloc();
	else
		msqkptr->label = NULL;
}

static void
mac_sysv_msgmsg_label_free(struct label *label)
{

	MAC_POLICY_PERFORM_NOSLEEP(sysvmsg_destroy_label, label);
	mac_labelzone_free(label);
}

void
mac_sysvmsg_destroy(struct msg *msgptr)
{

	if (msgptr->label != NULL) {
		mac_sysv_msgmsg_label_free(msgptr->label);
		msgptr->label = NULL;
	}
}

static void
mac_sysv_msgqueue_label_free(struct label *label)
{

	MAC_POLICY_PERFORM_NOSLEEP(sysvmsq_destroy_label, label);
	mac_labelzone_free(label);
}

void
mac_sysvmsq_destroy(struct msqid_kernel *msqkptr)
{

	if (msqkptr->label != NULL) {
		mac_sysv_msgqueue_label_free(msqkptr->label);
		msqkptr->label = NULL;
	}
}

void
mac_sysvmsg_create(struct ucred *cred, struct msqid_kernel *msqkptr,
    struct msg *msgptr)
{

	MAC_POLICY_PERFORM_NOSLEEP(sysvmsg_create, cred, msqkptr,
	    msqkptr->label, msgptr, msgptr->label);
}

void
mac_sysvmsq_create(struct ucred *cred, struct msqid_kernel *msqkptr)
{

	MAC_POLICY_PERFORM_NOSLEEP(sysvmsq_create, cred, msqkptr,
	    msqkptr->label);
}

void
mac_sysvmsg_cleanup(struct msg *msgptr)
{

	MAC_POLICY_PERFORM_NOSLEEP(sysvmsg_cleanup, msgptr->label);
}

void
mac_sysvmsq_cleanup(struct msqid_kernel *msqkptr)
{

	MAC_POLICY_PERFORM_NOSLEEP(sysvmsq_cleanup, msqkptr->label);
}

MAC_CHECK_PROBE_DEFINE3(sysvmsq_check_msgmsq, "struct ucred *",
    "struct msg *", "struct msqid_kernel *");

int
mac_sysvmsq_check_msgmsq(struct ucred *cred, struct msg *msgptr,
	struct msqid_kernel *msqkptr)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(sysvmsq_check_msgmsq, cred, msgptr,
	    msgptr->label, msqkptr, msqkptr->label);
	MAC_CHECK_PROBE3(sysvmsq_check_msgmsq, error, cred, msgptr, msqkptr);

	return (error);
}

MAC_CHECK_PROBE_DEFINE2(sysvmsq_check_msgrcv, "struct ucred *",
    "struct msg *");

int
mac_sysvmsq_check_msgrcv(struct ucred *cred, struct msg *msgptr)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(sysvmsq_check_msgrcv, cred, msgptr,
	    msgptr->label);
	MAC_CHECK_PROBE2(sysvmsq_check_msgrcv, error, cred, msgptr);

	return (error);
}

MAC_CHECK_PROBE_DEFINE2(sysvmsq_check_msgrmid, "struct ucred *",
    "struct msg *");

int
mac_sysvmsq_check_msgrmid(struct ucred *cred, struct msg *msgptr)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(sysvmsq_check_msgrmid, cred, msgptr,
	    msgptr->label);
	MAC_CHECK_PROBE2(sysvmsq_check_msgrmid, error, cred, msgptr);

	return (error);
}

MAC_CHECK_PROBE_DEFINE2(sysvmsq_check_msqget, "struct ucred *",
    "struct msqid_kernel *");

int
mac_sysvmsq_check_msqget(struct ucred *cred, struct msqid_kernel *msqkptr)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(sysvmsq_check_msqget, cred, msqkptr,
	    msqkptr->label);
	MAC_CHECK_PROBE2(sysvmsq_check_msqget, error, cred, msqkptr);

	return (error);
}

MAC_CHECK_PROBE_DEFINE2(sysvmsq_check_msqsnd, "struct ucred *",
    "struct msqid_kernel *");

int
mac_sysvmsq_check_msqsnd(struct ucred *cred, struct msqid_kernel *msqkptr)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(sysvmsq_check_msqsnd, cred, msqkptr,
	    msqkptr->label);
	MAC_CHECK_PROBE2(sysvmsq_check_msqsnd, error, cred, msqkptr);

	return (error);
}

MAC_CHECK_PROBE_DEFINE2(sysvmsq_check_msqrcv, "struct ucred *",
    "struct msqid_kernel *");

int
mac_sysvmsq_check_msqrcv(struct ucred *cred, struct msqid_kernel *msqkptr)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(sysvmsq_check_msqrcv, cred, msqkptr,
	    msqkptr->label);
	MAC_CHECK_PROBE2(sysvmsq_check_msqrcv, error, cred, msqkptr);

	return (error);
}

MAC_CHECK_PROBE_DEFINE3(sysvmsq_check_msqctl, "struct ucred *",
    "struct msqid_kernel *", "int");

int
mac_sysvmsq_check_msqctl(struct ucred *cred, struct msqid_kernel *msqkptr,
    int cmd)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(sysvmsq_check_msqctl, cred, msqkptr,
	    msqkptr->label, cmd);
	MAC_CHECK_PROBE3(sysvmsq_check_msqctl, error, cred, msqkptr, cmd);

	return (error);
}