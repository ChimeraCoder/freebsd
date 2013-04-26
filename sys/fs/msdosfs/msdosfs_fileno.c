
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mount.h>

#include <fs/msdosfs/bpb.h>
#include <fs/msdosfs/direntry.h>
#include <fs/msdosfs/msdosfsmount.h>

static MALLOC_DEFINE(M_MSDOSFSFILENO, "msdosfs_fileno", "MSDOSFS fileno mapping node");

RB_PROTOTYPE(msdosfs_filenotree, msdosfs_fileno, mf_tree,
    msdosfs_fileno_compare)

static int msdosfs_fileno_compare(struct msdosfs_fileno *,
    struct msdosfs_fileno *);

#define	FILENO_FIRST_DYN	0xf0000000

/* Initialize file number mapping structures. */
void
msdosfs_fileno_init(mp)
	struct mount *mp;
{
	struct msdosfsmount *pmp = VFSTOMSDOSFS(mp);

	RB_INIT(&pmp->pm_filenos);
	pmp->pm_nfileno = FILENO_FIRST_DYN;
        if (pmp->pm_HugeSectors > 0xffffffff /
	    (pmp->pm_BytesPerSec / sizeof(struct direntry)) + 1)
		pmp->pm_flags |= MSDOSFS_LARGEFS;
}

/* Free 32-bit file number generation structures. */
void
msdosfs_fileno_free(mp)
	struct mount *mp;
{
	struct msdosfsmount *pmp = VFSTOMSDOSFS(mp);
	struct msdosfs_fileno *mf, *next;

	for (mf = RB_MIN(msdosfs_filenotree, &pmp->pm_filenos); mf != NULL;
	    mf = next) {
		next = RB_NEXT(msdosfs_filenotree, &pmp->pm_filenos, mf);
		RB_REMOVE(msdosfs_filenotree, &pmp->pm_filenos, mf);
		free(mf, M_MSDOSFSFILENO);
	}
}

/* Map a 64-bit file number into a 32-bit one. */
uint32_t
msdosfs_fileno_map(mp, fileno)
	struct mount *mp;
	uint64_t fileno;
{
	struct msdosfsmount *pmp = VFSTOMSDOSFS(mp);
	struct msdosfs_fileno key, *mf, *tmf;
	uint32_t mapped;

	if ((pmp->pm_flags & MSDOSFS_LARGEFS) == 0) {
		KASSERT((uint32_t)fileno == fileno,
		    ("fileno >32 bits but not a large fs?"));
		return ((uint32_t)fileno);
	}
	if (fileno < FILENO_FIRST_DYN)
		return ((uint32_t)fileno);
	MSDOSFS_LOCK_MP(pmp);
	key.mf_fileno64 = fileno;
	mf = RB_FIND(msdosfs_filenotree, &pmp->pm_filenos, &key);
	if (mf != NULL) {
		mapped = mf->mf_fileno32;
		MSDOSFS_UNLOCK_MP(pmp);
		return (mapped);
	}
	if (pmp->pm_nfileno < FILENO_FIRST_DYN)
		panic("msdosfs_fileno_map: wraparound");
	MSDOSFS_UNLOCK_MP(pmp);
	mf = malloc(sizeof(*mf), M_MSDOSFSFILENO, M_WAITOK);
	MSDOSFS_LOCK_MP(pmp);
	tmf = RB_FIND(msdosfs_filenotree, &pmp->pm_filenos, &key);
	if (tmf != NULL) {
		mapped = tmf->mf_fileno32;
		MSDOSFS_UNLOCK_MP(pmp);
		free(mf, M_MSDOSFSFILENO);
		return (mapped);
	}
	mf->mf_fileno64 = fileno;
	mapped = mf->mf_fileno32 = pmp->pm_nfileno++;
	RB_INSERT(msdosfs_filenotree, &pmp->pm_filenos, mf);
	MSDOSFS_UNLOCK_MP(pmp);
	return (mapped);
}

/* Compare by 64-bit file number. */
static int
msdosfs_fileno_compare(fa, fb)
	struct msdosfs_fileno *fa, *fb;
{

	if (fa->mf_fileno64 > fb->mf_fileno64)
		return (1);
	else if (fa->mf_fileno64 < fb->mf_fileno64)
		return (-1);
	return (0);
}

RB_GENERATE(msdosfs_filenotree, msdosfs_fileno, mf_tree,
    msdosfs_fileno_compare)