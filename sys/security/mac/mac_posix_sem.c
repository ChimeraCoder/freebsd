
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
#include "opt_posix.h"

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/ksem.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/sdt.h>
#include <sys/systm.h>
#include <sys/sysctl.h>

#include <security/mac/mac_framework.h>
#include <security/mac/mac_internal.h>
#include <security/mac/mac_policy.h>

static struct label *
mac_posixsem_label_alloc(void)
{
	struct label *label;

	label = mac_labelzone_alloc(M_WAITOK);
	MAC_POLICY_PERFORM(posixsem_init_label, label);
	return (label);
}

void
mac_posixsem_init(struct ksem *ks)
{

	if (mac_labeled & MPC_OBJECT_POSIXSEM)
		ks->ks_label = mac_posixsem_label_alloc();
	else
		ks->ks_label = NULL;
}

static void
mac_posixsem_label_free(struct label *label)
{

	MAC_POLICY_PERFORM_NOSLEEP(posixsem_destroy_label, label);
	mac_labelzone_free(label);
}

void
mac_posixsem_destroy(struct ksem *ks)
{

	if (ks->ks_label != NULL) {
		mac_posixsem_label_free(ks->ks_label);
		ks->ks_label = NULL;
	}
}

void
mac_posixsem_create(struct ucred *cred, struct ksem *ks)
{

	MAC_POLICY_PERFORM_NOSLEEP(posixsem_create, cred, ks, ks->ks_label);
}

MAC_CHECK_PROBE_DEFINE2(posixsem_check_open, "struct ucred *",
    "struct ksem *");

int
mac_posixsem_check_open(struct ucred *cred, struct ksem *ks)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(posixsem_check_open, cred, ks,
	    ks->ks_label);
	MAC_CHECK_PROBE2(posixsem_check_open, error, cred, ks);

	return (error);
}

MAC_CHECK_PROBE_DEFINE3(posixsem_check_getvalue, "struct ucred *",
    "struct ucred *", "struct ksem *");

int
mac_posixsem_check_getvalue(struct ucred *active_cred, struct ucred *file_cred,
    struct ksem *ks)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(posixsem_check_getvalue, active_cred,
	    file_cred, ks, ks->ks_label);
	MAC_CHECK_PROBE3(posixsem_check_getvalue, error, active_cred,
	    file_cred, ks);

	return (error);
}

MAC_CHECK_PROBE_DEFINE3(posixsem_check_post, "struct ucred *",
    "struct ucred *", "struct ksem *");

int
mac_posixsem_check_post(struct ucred *active_cred, struct ucred *file_cred,
    struct ksem *ks)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(posixsem_check_post, active_cred, file_cred,
	    ks, ks->ks_label);
	MAC_CHECK_PROBE3(posixsem_check_post, error, active_cred, file_cred,
	    ks);

	return (error);
}

MAC_CHECK_PROBE_DEFINE3(posixsem_check_stat, "struct ucred *",
    "struct ucred *", "struct ksem *");

int
mac_posixsem_check_stat(struct ucred *active_cred, struct ucred *file_cred,
    struct ksem *ks)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(posixsem_check_stat, active_cred, file_cred,
	    ks, ks->ks_label);
	MAC_CHECK_PROBE3(posixsem_check_stat, error, active_cred, file_cred,
	    ks);

	return (error);
}

MAC_CHECK_PROBE_DEFINE2(posixsem_check_unlink, "struct ucred *",
    "struct ksem *");

int
mac_posixsem_check_unlink(struct ucred *cred, struct ksem *ks)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(posixsem_check_unlink, cred, ks,
	    ks->ks_label);
	MAC_CHECK_PROBE2(posixsem_check_unlink, error, cred, ks);

	return (error);
}

MAC_CHECK_PROBE_DEFINE3(posixsem_check_wait, "struct ucred *",
    "struct ucred *", "struct ksem *");

int
mac_posixsem_check_wait(struct ucred *active_cred, struct ucred *file_cred,
    struct ksem *ks)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(posixsem_check_wait, active_cred, file_cred,
	    ks, ks->ks_label);
	MAC_CHECK_PROBE3(posixsem_check_wait, error, active_cred, file_cred,
	    ks);

	return (error);
}

MAC_CHECK_PROBE_DEFINE3(posixsem_check_setmode, "struct ucred *",
    "struct ksem *", "mode_t");

int
mac_posixsem_check_setmode(struct ucred *cred, struct ksem *ks, mode_t mode)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(posixsem_check_setmode, cred, ks,
	    ks->ks_label, mode);
	MAC_CHECK_PROBE3(posixsem_check_setmode, error, cred, ks, mode);

	return (error);
}

MAC_CHECK_PROBE_DEFINE4(posixsem_check_setowner, "struct ucred *",
    "struct ks *", "uid_t", "gid_t");

int
mac_posixsem_check_setowner(struct ucred *cred, struct ksem *ks, uid_t uid,
    gid_t gid)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(posixsem_check_setowner, cred, ks,
	    ks->ks_label, uid, gid);
	MAC_CHECK_PROBE4(posixsem_check_setowner, error, cred, ks,
	    uid, gid);

	return (error);
}