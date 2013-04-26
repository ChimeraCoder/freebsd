
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
#include <sys/sem.h>

#include <security/mac/mac_framework.h>
#include <security/mac/mac_internal.h>
#include <security/mac/mac_policy.h>

static struct label *
mac_sysv_sem_label_alloc(void)
{
	struct label *label;

	label = mac_labelzone_alloc(M_WAITOK);
	MAC_POLICY_PERFORM(sysvsem_init_label, label);
	return (label);
}

void
mac_sysvsem_init(struct semid_kernel *semakptr)
{

	if (mac_labeled & MPC_OBJECT_SYSVSEM)
		semakptr->label = mac_sysv_sem_label_alloc();
	else
		semakptr->label = NULL;
}

static void
mac_sysv_sem_label_free(struct label *label)
{

	MAC_POLICY_PERFORM_NOSLEEP(sysvsem_destroy_label, label);
	mac_labelzone_free(label);
}

void
mac_sysvsem_destroy(struct semid_kernel *semakptr)
{

	if (semakptr->label != NULL) {
		mac_sysv_sem_label_free(semakptr->label);
		semakptr->label = NULL;
	}
}

void
mac_sysvsem_create(struct ucred *cred, struct semid_kernel *semakptr)
{

	MAC_POLICY_PERFORM_NOSLEEP(sysvsem_create, cred, semakptr,
	    semakptr->label);
}

void
mac_sysvsem_cleanup(struct semid_kernel *semakptr)
{

	MAC_POLICY_PERFORM_NOSLEEP(sysvsem_cleanup, semakptr->label);
}

MAC_CHECK_PROBE_DEFINE3(sysvsem_check_semctl, "struct ucred *",
    "struct semid_kernel *", "int");

int
mac_sysvsem_check_semctl(struct ucred *cred, struct semid_kernel *semakptr,
    int cmd)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(sysvsem_check_semctl, cred, semakptr,
	    semakptr->label, cmd);
	MAC_CHECK_PROBE3(sysvsem_check_semctl, error, cred, semakptr, cmd);

	return (error);
}

MAC_CHECK_PROBE_DEFINE2(sysvsem_check_semget, "struct ucred *",
    "struct semid_kernel *");

int
mac_sysvsem_check_semget(struct ucred *cred, struct semid_kernel *semakptr)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(sysvsem_check_semget, cred, semakptr,
	    semakptr->label);

	return (error);
}

MAC_CHECK_PROBE_DEFINE3(sysvsem_check_semop, "struct ucred *",
    "struct semid_kernel *", "size_t");

int
mac_sysvsem_check_semop(struct ucred *cred, struct semid_kernel *semakptr,
    size_t accesstype)
{
	int error;

	MAC_POLICY_CHECK_NOSLEEP(sysvsem_check_semop, cred, semakptr,
	    semakptr->label, accesstype);
	MAC_CHECK_PROBE3(sysvsem_check_semop, error, cred, semakptr,
	    accesstype);

	return (error);
}