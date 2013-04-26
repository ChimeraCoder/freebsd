
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

#include "opt_pseudofs.h"

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/limits.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/sysctl.h>
#include <sys/systm.h>

#include <fs/pseudofs/pseudofs.h>
#include <fs/pseudofs/pseudofs_internal.h>

/*
 * Initialize fileno bitmap
 */
void
pfs_fileno_init(struct pfs_info *pi)
{

	mtx_assert(&Giant, MA_OWNED);
	mtx_init(&pi->pi_mutex, "pfs_fileno", NULL, MTX_DEF);
	pi->pi_unrhdr = new_unrhdr(3, INT_MAX / NO_PID, &pi->pi_mutex);
}

/*
 * Tear down fileno bitmap
 */
void
pfs_fileno_uninit(struct pfs_info *pi)
{

	mtx_assert(&Giant, MA_OWNED);
	delete_unrhdr(pi->pi_unrhdr);
	pi->pi_unrhdr = NULL;
	mtx_destroy(&pi->pi_mutex);
}

/*
 * Allocate a file number
 */
void
pfs_fileno_alloc(struct pfs_node *pn)
{

	if (pn->pn_parent)
		PFS_TRACE(("%s/%s", pn->pn_parent->pn_name, pn->pn_name));
	else
		PFS_TRACE(("%s", pn->pn_name));
	pfs_assert_not_owned(pn);

	switch (pn->pn_type) {
	case pfstype_root:
		/* root must always be 2 */
		pn->pn_fileno = 2;
		break;
	case pfstype_dir:
	case pfstype_file:
	case pfstype_symlink:
	case pfstype_procdir:
		pn->pn_fileno = alloc_unr(pn->pn_info->pi_unrhdr);
		break;
	case pfstype_this:
		KASSERT(pn->pn_parent != NULL,
		    ("%s(): pfstype_this node has no parent", __func__));
		pn->pn_fileno = pn->pn_parent->pn_fileno;
		break;
	case pfstype_parent:
		KASSERT(pn->pn_parent != NULL,
		    ("%s(): pfstype_parent node has no parent", __func__));
		if (pn->pn_parent->pn_type == pfstype_root) {
			pn->pn_fileno = pn->pn_parent->pn_fileno;
			break;
		}
		KASSERT(pn->pn_parent->pn_parent != NULL,
		    ("%s(): pfstype_parent node has no grandparent", __func__));
		pn->pn_fileno = pn->pn_parent->pn_parent->pn_fileno;
		break;
	case pfstype_none:
		KASSERT(0,
		    ("%s(): pfstype_none node", __func__));
		break;
	}

#if 0
	printf("%s(): %s: ", __func__, pn->pn_info->pi_name);
	if (pn->pn_parent) {
		if (pn->pn_parent->pn_parent) {
			printf("%s/", pn->pn_parent->pn_parent->pn_name);
		}
		printf("%s/", pn->pn_parent->pn_name);
	}
	printf("%s -> %d\n", pn->pn_name, pn->pn_fileno);
#endif
}

/*
 * Release a file number
 */
void
pfs_fileno_free(struct pfs_node *pn)
{

	pfs_assert_not_owned(pn);

	switch (pn->pn_type) {
	case pfstype_root:
		/* not allocated from unrhdr */
		return;
	case pfstype_dir:
	case pfstype_file:
	case pfstype_symlink:
	case pfstype_procdir:
		free_unr(pn->pn_info->pi_unrhdr, pn->pn_fileno);
		break;
	case pfstype_this:
	case pfstype_parent:
		/* ignore these, as they don't "own" their file number */
		break;
	case pfstype_none:
		KASSERT(0,
		    ("pfs_fileno_free() called for pfstype_none node"));
		break;
	}
}