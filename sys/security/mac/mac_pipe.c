
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
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/sbuf.h>
#include <sys/sdt.h>
#include <sys/systm.h>
#include <sys/vnode.h>
#include <sys/pipe.h>
#include <sys/sysctl.h>

#include <security/mac/mac_framework.h>
#include <security/mac/mac_internal.h>
#include <security/mac/mac_policy.h>

struct label *
mac_pipe_label_alloc(void)
{
	struct label *label;

	label = mac_labelzone_alloc(M_WAITOK);
	MAC_POLICY_PERFORM(pipe_init_label, label);
	return (label);
}

void
mac_pipe_init(struct pipepair *pp)
{

	if (mac_labeled & MPC_OBJECT_PIPE)
		pp->pp_label = mac_pipe_label_alloc();
	else
		pp->pp_label = NULL;
}

void
mac_pipe_label_free(struct label *label)
{

	MAC_POLICY_PERFORM_NOSLEEP(pipe_destroy_label, label);
	mac_labelzone_free(label);
}

void
mac_pipe_destroy(struct pipepair *pp)
{

	if (pp->pp_label != NULL) {
		mac_pipe_label_free(pp->pp_label);
		pp->pp_label = NULL;
	}
}

void
mac_pipe_copy_label(struct label *src, struct label *dest)
{

	MAC_POLICY_PERFORM_NOSLEEP(pipe_copy_label, src, dest);
}

int
mac_pipe_externalize_label(struct label *label, char *elements,
    char *outbuf, size_t outbuflen)
{
	int error;

	MAC_POLICY_EXTERNALIZE(pipe, label, elements, outbuf, outbuflen);

	return (error);
}

int
mac_pipe_internalize_label(struct label *label, char *string)
{
	int error;

	MAC_POLICY_INTERNALIZE(pipe, label, string);

	return (error);
}

void
mac_pipe_create(struct ucred *cred, struct pipepair *pp)
{

	MAC_POLICY_PERFORM_NOSLEEP(pipe_create, cred, pp, pp->pp_label);
}

static void
mac_pipe_relabel(struct ucred *cred, struct pipepair *pp,
    struct label *newlabel)
{

	MAC_POLICY_PERFORM_NOSLEEP(pipe_relabel, cred, pp, pp->pp_label,
	    newlabel);
}

MAC_CHECK_PROBE_DEFINE4(pipe_check_ioctl, "struct ucred *",
    "struct pipepair *", "unsigned long", "void *");

int
mac_pipe_check_ioctl(struct ucred *cred, struct pipepair *pp,
    unsigned long cmd, void *data)
{
	int error;

	mtx_assert(&pp->pp_mtx, MA_OWNED);

	MAC_POLICY_CHECK_NOSLEEP(pipe_check_ioctl, cred, pp, pp->pp_label,
	    cmd, data);
	MAC_CHECK_PROBE4(pipe_check_ioctl, error, cred, pp, cmd, data);

	return (error);
}

MAC_CHECK_PROBE_DEFINE2(pipe_check_poll, "struct ucred *",
    "struct pipepair *");

int
mac_pipe_check_poll(struct ucred *cred, struct pipepair *pp)
{
	int error;

	mtx_assert(&pp->pp_mtx, MA_OWNED);

	MAC_POLICY_CHECK_NOSLEEP(pipe_check_poll, cred, pp, pp->pp_label);
	MAC_CHECK_PROBE2(pipe_check_poll, error, cred, pp);

	return (error);
}

MAC_CHECK_PROBE_DEFINE2(pipe_check_read, "struct ucred *",
    "struct pipepair *");

int
mac_pipe_check_read(struct ucred *cred, struct pipepair *pp)
{
	int error;

	mtx_assert(&pp->pp_mtx, MA_OWNED);

	MAC_POLICY_CHECK_NOSLEEP(pipe_check_read, cred, pp, pp->pp_label);
	MAC_CHECK_PROBE2(pipe_check_read, error, cred, pp);

	return (error);
}

MAC_CHECK_PROBE_DEFINE3(pipe_check_relabel, "struct ucred *",
    "struct pipepair *", "struct label *");

static int
mac_pipe_check_relabel(struct ucred *cred, struct pipepair *pp,
    struct label *newlabel)
{
	int error;

	mtx_assert(&pp->pp_mtx, MA_OWNED);

	MAC_POLICY_CHECK_NOSLEEP(pipe_check_relabel, cred, pp, pp->pp_label,
	    newlabel);
	MAC_CHECK_PROBE3(pipe_check_relabel, error, cred, pp, newlabel);

	return (error);
}

MAC_CHECK_PROBE_DEFINE2(pipe_check_stat, "struct ucred *",
    "struct pipepair *");

int
mac_pipe_check_stat(struct ucred *cred, struct pipepair *pp)
{
	int error;

	mtx_assert(&pp->pp_mtx, MA_OWNED);

	MAC_POLICY_CHECK_NOSLEEP(pipe_check_stat, cred, pp, pp->pp_label);
	MAC_CHECK_PROBE2(pipe_check_stat, error, cred, pp);

	return (error);
}

MAC_CHECK_PROBE_DEFINE2(pipe_check_write, "struct ucred *",
    "struct pipepair *");

int
mac_pipe_check_write(struct ucred *cred, struct pipepair *pp)
{
	int error;

	mtx_assert(&pp->pp_mtx, MA_OWNED);

	MAC_POLICY_CHECK_NOSLEEP(pipe_check_write, cred, pp, pp->pp_label);
	MAC_CHECK_PROBE2(pipe_check_write, error, cred, pp);

	return (error);
}

int
mac_pipe_label_set(struct ucred *cred, struct pipepair *pp,
    struct label *label)
{
	int error;

	mtx_assert(&pp->pp_mtx, MA_OWNED);

	error = mac_pipe_check_relabel(cred, pp, label);
	if (error)
		return (error);

	mac_pipe_relabel(cred, pp, label);

	return (0);
}