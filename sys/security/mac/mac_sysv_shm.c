
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
#include <sys/systm.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/file.h>
#include <sys/namei.h>
#include <sys/sdt.h>
#include <sys/sysctl.h>
#include <sys/shm.h>

#include <security/mac/mac_framework.h>
#include <security/mac/mac_internal.h>
#include <security/mac/mac_policy.h>

static struct label *
mac_sysv_shm_label_alloc(void)
{
	struct label *label;

	label = mac_labelzone_alloc(M_WAITOK);
	MAC_POLICY_PERFORM(sysvshm_init_label, label);
	return (label);
}

void
mac_sysvshm_init(struct shmid_kernel *shmsegptr)
{

	if (mac_labeled & MPC_OBJECT_SYSVSHM)
		shmsegptr->label = mac_sysv_shm_label_alloc();
	else
		shmsegptr->label = NULL;
}

static void
mac_sysv_shm_label_free(struct label *label)
{

	MAC_POLICY_PERFORM_NOSLEEP(sysvshm_destroy_label, label);
	mac_labelzone_free(label);
}

void
mac_sysvshm_destroy(struct shmid_kernel *shmsegptr)
{

	if (shmsegptr->label != NULL) {
		mac_sysv_shm_label_free(shmsegptr->label);
		shmsegptr->label = NULL;
	}
}

void
mac_sysvshm_create(struct ucred *cred, struct shmid_kernel *shmsegptr)
{

	MAC_POLICY_PERFORM_NOSLEEP(sysvshm_create, cred, shmsegptr,
	    shmsegptr->label);
}

void
mac_sysvshm_cleanup(struct shmid_kernel *shmsegptr)
{

	MAC_POLICY_PERFORM_NOSLEEP(sysvshm_cleanup, shmsegptr->label);
}

MAC_CHECK_PROBE_DEFINE3(sysvshm_check_shmat, "struct ucred *",
    "struct shmid_kernel *", "int");

int
mac_sysvshm_check_shmat(struct ucred *cred, struct shmid_kernel *shmsegptr,
    int shmflg)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(sysvshm_check_shmat, cred, shmsegptr,
	    shmsegptr->label, shmflg);
	MAC_CHECK_PROBE3(sysvshm_check_shmat, error, cred, shmsegptr,
	    shmflg);

	return (error);
}

MAC_CHECK_PROBE_DEFINE3(sysvshm_check_shmctl, "struct ucred *",
    "struct shmid_kernel *", "int");

int
mac_sysvshm_check_shmctl(struct ucred *cred, struct shmid_kernel *shmsegptr,
    int cmd)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(sysvshm_check_shmctl, cred, shmsegptr,
	    shmsegptr->label, cmd);
	MAC_CHECK_PROBE3(sysvshm_check_shmctl, error, cred, shmsegptr, cmd);

	return (error);
}

MAC_CHECK_PROBE_DEFINE2(sysvshm_check_shmdt, "struct ucred *",
    "struct shmid *");

int
mac_sysvshm_check_shmdt(struct ucred *cred, struct shmid_kernel *shmsegptr)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(sysvshm_check_shmdt, cred, shmsegptr,
	    shmsegptr->label);
	MAC_CHECK_PROBE2(sysvshm_check_shmdt, error, cred, shmsegptr);

	return (error);
}

MAC_CHECK_PROBE_DEFINE3(sysvshm_check_shmget, "struct ucred *",
    "struct shmid_kernel *", "int");

int
mac_sysvshm_check_shmget(struct ucred *cred, struct shmid_kernel *shmsegptr,
    int shmflg)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(sysvshm_check_shmget, cred, shmsegptr,
	    shmsegptr->label, shmflg);
	MAC_CHECK_PROBE3(sysvshm_check_shmget, error, cred, shmsegptr,
	    shmflg);

	return (error);
}